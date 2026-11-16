#!/usr/bin/env python3
"""
Cobertura RVV (instruções, SEW/LMUL, pares e triplas) de uma SEED inteira, isto é, dos 76
arquivos de entrada que formam uma seed no multi_run_paper.py:

    seed 001: 001..076.int32.in   seed 002: 077..152.int32.in   seed 003: 153..228.int32.in

O commit log de um arquivo tem ~730 MB (12 M linhas); uma seed passaria de 50 GB. Por isso
nada é gravado: o log do Spike (-l --log-commits, que sai no stderr) passa por um
`grep` que deixa só as linhas de instruções vetoriais e de commit vetorial (~3,5 MB) e é
lido direto do pipe. Os contadores de cada arquivo são somados aos da seed e salvos após
cada arquivo em OUT/<seed>/state.json (gravação atômica). Se a execução parar, rodar de
novo continua de onde parou (use --restart para começar do zero).

As contagens seguem o rvv_coverage.py (mesmo parser): pares/triplas são de instruções RVV
consecutivas, ignorando as escalares e as de --exclude. A janela recomeça a cada arquivo.

Tempo sem log: depois da cobertura, cada arquivo da seed roda de novo SEM -l (Spike normal,
só o programa), um por vez (--time-jobs 1) para o tempo não sofrer com a concorrência. O
tempo de cada arquivo vai para o state.json; o report.txt traz o total da seed (quanto levaria
para rodar a seed sem gerar log) e o tempo das execuções com log, para comparação.
Para medir só o tempo de uma seed já coberta: rode de novo (os arquivos já cobertos são pulados).
--no-time pula essa etapa.

Instruções retiradas: cada arquivo também roda com `spike -g` (histograma de PCs, sem log;
~2x o tempo normal); a soma do histograma é o total de instruções retiradas (escalares
inclusive). O report.txt traz o total da seed, por arquivo e por sequência, e quantas delas são
vetoriais (soma das contagens do log).

Saídas em OUT/<seed>/ (OUT padrão: benchmarks/BENCH/test_runs/coverage):
    state.json     contadores acumulados + arquivos já processados (checkpoint)
    progress.csv   cobertura acumulada após cada arquivo (curva de cobertura)
    report.txt     relatório no formato do rvv_coverage.py
    coverage.csv   por instrução;  pairs.csv / triples.csv  pares e triplas com contagem
OUT/all/ tem o mesmo para todas as seeds juntas.

Uso:
    python3 test_runs/rvv_coverage_seed.py [BENCH] [-s 001,003] [-j 8] [--restart]
    python3 test_runs/rvv_coverage_seed.py --report-only     # só refaz os relatórios
"""

import argparse
import collections
import csv
import json
import os
import shutil
import signal
import subprocess
import sys
import tempfile
import time
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from rvv_coverage import COMMIT_RE, DISASM_RE, load_reference, normalize  # noqa: E402

ISA = "rv64gcv_zvl128b_zicntr_zba_zbb"
SEQUENCES_PER_SEED = 10**5
TESTS_PER_FILE = 1332
FILES_PER_SEED = -(-SEQUENCES_PER_SEED // TESTS_PER_FILE)   # 76
# Disassembly de instrução vetorial ") vxxx" ou commit com estado vetorial ") e32 m1 l4"
LOG_FILTER = r"\) (v[a-z]|e[0-9])"


# ---------------------------------------------------------------------------
# Trabalho de um arquivo (roda em outro processo)

def cover_file(spike, base_riscv, input_file, reference, exclude, timeout):
    """Roda o Spike com log em pipe e devolve os contadores do arquivo."""
    counts = collections.Counter()
    configs = collections.defaultdict(collections.Counter)
    pairs = collections.Counter()
    triples = collections.Counter()
    window = collections.deque(maxlen=3)
    last = None

    with tempfile.TemporaryDirectory(prefix="rvvcov_") as tmp:
        riscv = Path(tmp) / "prog.riscv"
        shutil.copy2(base_riscv, riscv)
        subprocess.run(["riscv64-unknown-elf-objcopy", "--update-section",
                        f".OUT_SECTION={input_file}", str(riscv)], check=True)

        start = time.monotonic()
        spike_p = subprocess.Popen([spike, "-l", "--log-commits", f"--isa={ISA}", str(riscv)],
                                   stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        grep_p = subprocess.Popen(["grep", "-E", LOG_FILTER], stdin=spike_p.stderr,
                                  stdout=subprocess.PIPE, env={**os.environ, "LC_ALL": "C"})
        spike_p.stderr.close()
        try:
            for raw in grep_p.stdout:
                line = raw.decode("ascii", errors="replace")
                m = COMMIT_RE.match(line)
                if m and last:
                    configs[last][f"{int(m.group(1))}/{m.group(2)}"] += 1
                    continue
                m = DISASM_RE.match(line)
                if m:
                    name = normalize(m.group(1))
                    last = name if name in reference else None
                    if last:
                        counts[last] += 1
                        if last not in exclude:
                            window.append(last)
                            if len(window) >= 2:
                                pairs[",".join(list(window)[-2:])] += 1
                            if len(window) == 3:
                                triples[",".join(window)] += 1
                if time.monotonic() - start > timeout:
                    raise TimeoutError
            rc = spike_p.wait(timeout=max(1, timeout - (time.monotonic() - start)))
        except (TimeoutError, subprocess.TimeoutExpired):
            spike_p.kill()
            rc = "timeout"
        finally:
            grep_p.stdout.close()
            grep_p.wait()
            if spike_p.poll() is None:
                spike_p.kill()
                spike_p.wait()
        seconds = round(time.monotonic() - start, 2)
        retired = retired_instructions(spike, riscv, timeout)

    return {
        "file": input_file.name, "rc": rc, "seconds": seconds, "retired": retired,
        "counts": dict(counts), "configs": {k: dict(v) for k, v in configs.items()},
        "pairs": dict(pairs), "triples": dict(triples),
    }


def retired_instructions(spike, riscv, timeout):
    """Total de instruções retiradas (soma do histograma do spike -g), ou None."""
    try:
        r = subprocess.run([spike, "-g", f"--isa={ISA}", str(riscv)], stdout=subprocess.DEVNULL,
                           stderr=subprocess.PIPE, timeout=timeout)
    except subprocess.TimeoutExpired:
        return None
    total = 0
    for line in r.stderr.decode(errors="replace").splitlines():
        parts = line.split()
        if len(parts) == 2 and parts[1].isdigit():
            total += int(parts[1])
    return total


def time_file(spike, base_riscv, input_file, timeout):
    """Tempo do arquivo no Spike sem log (só o programa). Retorna (arquivo, rc, segundos)."""
    with tempfile.TemporaryDirectory(prefix="rvvtime_") as tmp:
        riscv = Path(tmp) / "prog.riscv"
        shutil.copy2(base_riscv, riscv)
        subprocess.run(["riscv64-unknown-elf-objcopy", "--update-section",
                        f".OUT_SECTION={input_file}", str(riscv)], check=True)
        start = time.monotonic()
        try:
            rc = subprocess.run([spike, f"--isa={ISA}", str(riscv)], stdout=subprocess.DEVNULL,
                                stderr=subprocess.DEVNULL, timeout=timeout).returncode
        except subprocess.TimeoutExpired:
            rc = "timeout"
        return input_file.name, rc, time.monotonic() - start


def load_log_seconds(progress_path):
    """{arquivo: segundos com log} do progress.csv (estados antigos não guardavam)."""
    result = {}
    if progress_path.is_file():
        with open(progress_path, newline="") as f:
            for row in csv.DictReader(f):
                try:
                    result[row["file"]] = float(row["seconds"])
                except (KeyError, ValueError):
                    pass
    return result


# ---------------------------------------------------------------------------
# Estado acumulado

def empty_state():
    return {"files": [], "counts": {}, "configs": {}, "pairs": {}, "triples": {}}


def merge(state, part):
    for key in ("counts", "pairs", "triples"):
        dst = state[key]
        for k, v in part[key].items():
            dst[k] = dst.get(k, 0) + v
    for insn, bins in part["configs"].items():
        dst = state["configs"].setdefault(insn, {})
        for k, v in bins.items():
            dst[k] = dst.get(k, 0) + v
    state["files"].append(part["file"])
    if "seconds" in part:
        state.setdefault("log_seconds", {})[part["file"]] = part["seconds"]
    if part.get("retired") is not None:
        state.setdefault("retired", {})[part["file"]] = part["retired"]


def save_json(path, data):
    tmp = path.with_suffix(".tmp")
    tmp.write_text(json.dumps(data, separators=(",", ":")))
    os.replace(tmp, path)


def load_state(path):
    if path.is_file():
        return json.loads(path.read_text())
    return empty_state()


def write_report(state, out_dir, groups, reference, exclude, title):
    """report.txt, coverage.csv, pairs.csv e triples.csv a partir do estado."""
    counts = collections.Counter(state["counts"])
    configs = state["configs"]
    pairs = collections.Counter({tuple(k.split(",")): v for k, v in state["pairs"].items()})
    triples = collections.Counter({tuple(k.split(",")): v for k, v in state["triples"].items()})

    lines = [f"{title}: {len(state['files'])} arquivos "
             f"(~{len(state['files']) * TESTS_PER_FILE} sequências)", ""]
    lines.append(f"{'group':28s} {'covered':>8s} {'total':>6s} {'%':>7s}")
    for g, insns in groups.items():
        hit = sum(1 for i in insns if counts[i])
        lines.append(f"{g:28s} {hit:8d} {len(insns):6d} {100 * hit / max(len(insns), 1):6.1f}%")
    hit = sum(1 for i in reference if counts[i])
    lines.append(f"{'TOTAL':28s} {hit:8d} {len(reference):6d} {100 * hit / len(reference):6.1f}%")
    lines.append("")

    covered = sorted(i for i in counts if i not in exclude)
    n = len(covered)
    pair_first = collections.Counter(a for a, _ in pairs)
    triple_first = collections.Counter(a for a, _, _ in triples)
    lines.append(f"Pair/triple coverage over the {n} covered instructions"
                 + (f" (excluding {', '.join(sorted(exclude))})" if exclude else "") + ":")
    lines.append(f"  pairs   {len(pairs):10d} / {n ** 2:12d}  {100 * len(pairs) / max(n ** 2, 1):8.4f}%"
                 f"  ({sum(pairs.values())} occurrences)")
    lines.append(f"  triples {len(triples):10d} / {n ** 3:12d}  {100 * len(triples) / max(n ** 3, 1):8.4f}%"
                 f"  ({sum(triples.values())} occurrences)")
    lines.append("")
    retired = state.get("retired", {})
    if retired:
        total = sum(retired.values())
        vector = sum(counts.values())
        lines.append(f"Instruções retiradas (spike -g, escalares inclusive): {len(retired)} arquivos")
        lines.append(f"  total {total:,}   média {total / len(retired):,.0f} por arquivo"
                     f"   {total / (len(retired) * TESTS_PER_FILE):,.0f} por sequência"
                     .replace(",", "."))
        lines.append(f"  vetoriais (log): {vector:,} ({100 * vector / total:.1f}% do total)"
                     .replace(",", "."))
        lines.append("")
    plain = state.get("plain_seconds", {})
    logged = state.get("log_seconds", {})
    if plain:
        total = sum(plain.values())
        lines.append(f"Tempo sem log (Spike normal, um arquivo por vez): {len(plain)} arquivos")
        lines.append(f"  total {total:10.1f} s  ({total / 60:.1f} min)   média {total / len(plain):.2f} s/arquivo"
                     f"   ({1000 * total / (len(plain) * TESTS_PER_FILE):.2f} ms/sequência)")
    if logged:
        total = sum(logged.values())
        lines.append(f"Tempo com log (-l --log-commits + grep, arquivos em paralelo): {len(logged)} arquivos")
        lines.append(f"  total {total:10.1f} s  ({total / 60:.1f} min)   média {total / len(logged):.2f} s/arquivo")
    if plain and logged:
        common = set(plain) & set(logged)
        if common:
            ratio = sum(logged[f] for f in common) / max(sum(plain[f] for f in common), 1e-9)
            lines.append(f"  com log / sem log: {ratio:.1f}x")
    if plain or logged:
        lines.append("")
    lines.append(f"Executed instructions (count, pairs as first /{n}, triples as first /{n ** 2}, "
                 "SEW/LMUL bins):")
    for insn, c in counts.most_common():
        bins = ", ".join(f"e{b}" for b in sorted(configs.get(insn, {})))
        lines.append(f"  {insn:20s} {c:12d} {pair_first[insn]:6d} {triple_first[insn]:8d}  {bins}")

    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "report.txt").write_text("\n".join(lines) + "\n")

    with open(out_dir / "coverage.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["group", "instruction", "count", "pairs_as_first", "triples_as_first",
                    "sew_lmul_bins"])
        for insn, g in reference.items():
            bins = ";".join(f"e{b}" for b in sorted(configs.get(insn, {})))
            w.writerow([g, insn, counts[insn], pair_first[insn], triple_first[insn], bins])
    for name, grams, cols in (("pairs.csv", pairs, ["first", "second"]),
                              ("triples.csv", triples, ["first", "second", "third"])):
        with open(out_dir / name, "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(cols + ["count"])
            for gram, c in grams.most_common():
                w.writerow(list(gram) + [c])

    return hit, len(reference), n, len(pairs), len(triples)


def summary_line(state, reference, exclude):
    counts = state["counts"]
    n = sum(1 for i in counts if i not in exclude)
    return (sum(1 for i in reference if counts.get(i)), n, len(state["pairs"]),
            len(state["triples"]))


# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Cobertura RVV de seeds inteiras (76 arquivos), sem guardar os logs.",
        formatter_class=argparse.RawDescriptionHelpFormatter, epilog=__doc__)
    parser.add_argument("bench", nargs="?", default="random_neural_network")
    parser.add_argument("bits", nargs="?", default="32")
    parser.add_argument("-s", "--seeds", default="001,002,003",
                        help="seeds separadas por vírgula (padrão: 001,002,003)")
    parser.add_argument("--dist", default="uniform")
    parser.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 2) // 2),
                        help="arquivos em paralelo (cada um usa Spike + grep; padrão: núcleos/2)")
    parser.add_argument("--timeout", type=float, default=600, help="segundos por arquivo")
    parser.add_argument("--exclude", default="vsetvli,vle32_v,vse32_v",
                        help="instruções fora dos pares/triplas (padrão: vsetvli,vle32_v,vse32_v)")
    parser.add_argument("--spike", default="spike")
    parser.add_argument("--spike-src", default=os.path.expanduser("~/Documents/Mestrado/riscv-isa-sim"))
    parser.add_argument("-o", "--out-dir", help="padrão: benchmarks/BENCH/test_runs/coverage")
    parser.add_argument("--no-time", action="store_true",
                        help="não mede o tempo sem log")
    parser.add_argument("--time-jobs", type=int, default=1,
                        help="arquivos em paralelo ao medir o tempo sem log (padrão: 1)")
    parser.add_argument("--max-files", type=int, help="só os N primeiros arquivos de cada seed (teste)")
    parser.add_argument("--restart", action="store_true", help="ignora o checkpoint das seeds")
    parser.add_argument("--report-only", action="store_true",
                        help="não roda nada; refaz os relatórios a partir dos state.json")
    args = parser.parse_args()

    out_dir = Path(args.out_dir).resolve() if args.out_dir else None
    if Path.cwd().name == "test_runs":
        os.chdir("..")
    bench = args.bench
    out_root = out_dir or Path(f"./benchmarks/{bench}/test_runs/coverage")

    groups = load_reference(args.spike_src)
    reference = {i: g for g, insns in groups.items() for i in insns}
    exclude = {normalize(i.strip()) for i in args.exclude.split(",") if i.strip()}

    input_dir = Path(f"./test_runs/{args.dist}/inputs{args.bits}")
    all_inputs = sorted(input_dir.glob(f"*.int{args.bits}.in"))
    seed_files = {f"{k + 1:03d}": all_inputs[k * FILES_PER_SEED:(k + 1) * FILES_PER_SEED]
                  for k in range(len(all_inputs) // FILES_PER_SEED)}
    seeds = [s.strip().zfill(3) for s in args.seeds.split(",") if s.strip()]
    for s in seeds:
        if s not in seed_files:
            sys.exit(f"Seed {s} não existe ({len(all_inputs)} arquivos int{args.bits} em {input_dir}).")

    if not args.report_only:
        # Binário base com os parâmetros; a .OUT_SECTION é trocada por arquivo
        subprocess.run(["python3", "./mem_setter.py", f"./benchmarks/{bench}/params.json"],
                       check=True, stdout=subprocess.DEVNULL)
        base_riscv = out_root / f"{bench}.base.riscv"
        out_root.mkdir(parents=True, exist_ok=True)
        shutil.copy2(f"./benchmarks/{bench}.riscv", base_riscv)
        subprocess.run(["riscv64-unknown-elf-objcopy", "--update-section",
                        f".PARAMETERS_SECTION=./benchmarks/{bench}/params.mem", str(base_riscv)],
                       check=True)

        executor = ProcessPoolExecutor(max_workers=args.jobs)
        try:
            for seed in seeds:
                seed_dir = out_root / seed
                seed_dir.mkdir(parents=True, exist_ok=True)
                state_path = seed_dir / "state.json"
                progress_path = seed_dir / "progress.csv"
                if args.restart:
                    state_path.unlink(missing_ok=True)
                    progress_path.unlink(missing_ok=True)
                state = load_state(state_path)
                done = set(state["files"])
                files = seed_files[seed][:args.max_files]
                todo = [f for f in files if f.name not in done]
                print(f"Seed {seed}: {files[0].name} .. {files[-1].name} — "
                      f"{len(done)} já feitos, {len(todo)} a rodar")
                if not progress_path.exists():
                    progress_path.write_text("files_done,file,rc,seconds,instructions_covered,"
                                             "covered_for_pairs,pairs,triples\n")

                futures = {executor.submit(cover_file, args.spike, base_riscv, f.resolve(),
                                           reference, exclude, args.timeout): f for f in todo}
                start = time.monotonic()
                for k, future in enumerate(as_completed(futures), 1):
                    part = future.result()
                    if part["rc"] != 0:
                        print(f"  [aviso] {part['file']}: rc={part['rc']} (contado mesmo assim)")
                    merge(state, part)
                    save_json(state_path, state)
                    instr, n, p, t = summary_line(state, reference, exclude)
                    with open(progress_path, "a") as f:
                        f.write(f"{len(state['files'])},{part['file']},{part['rc']},"
                                f"{part['seconds']},{instr},{n},{p},{t}\n")
                    elapsed = time.monotonic() - start
                    eta = elapsed / k * (len(todo) - k)
                    print(f"  [{len(state['files']):>2}/{len(files)}] {part['file']} "
                          f"{part['seconds']:.0f}s  instr={instr} pares={p} triplas={t}  "
                          f"(faltam ~{eta / 60:.0f} min)", flush=True)
                # Estados antigos: tempo com log vem do progress.csv
                if len(state.get("log_seconds", {})) < len(state["files"]):
                    state.setdefault("log_seconds", {}).update(
                        {k: v for k, v in load_log_seconds(progress_path).items()
                         if k not in state["log_seconds"]})
                    save_json(state_path, state)

                if not args.no_time:
                    plain = state.setdefault("plain_seconds", {})
                    to_time = [f for f in files if f.name not in plain]
                    if to_time:
                        print(f"Seed {seed}: medindo tempo sem log de {len(to_time)} arquivos "
                              f"({args.time_jobs} por vez)")
                    with ProcessPoolExecutor(max_workers=args.time_jobs) as timer:
                        jobs = [timer.submit(time_file, args.spike, base_riscv, f.resolve(),
                                             args.timeout) for f in to_time]
                        for k, job in enumerate(as_completed(jobs), 1):
                            name, rc, seconds = job.result()
                            if rc != 0:
                                print(f"  [aviso] {name}: rc={rc} sem log")
                            plain[name] = round(seconds, 3)
                            save_json(state_path, state)
                            if k % 10 == 0 or k == len(to_time):
                                print(f"  [{k}/{len(to_time)}] soma até agora "
                                      f"{sum(plain.values()):.1f}s", flush=True)

                write_report(state, seed_dir, groups, reference, exclude, f"Seed {seed}")
        except KeyboardInterrupt:
            print("\nInterrompido; o progresso está salvo. Rode de novo para continuar.")
            executor.shutdown(wait=False, cancel_futures=True)
            subprocess.run(["pkill", "-f", str(base_riscv)], check=False)
            sys.exit(130)
        executor.shutdown()
        base_riscv.unlink(missing_ok=True)

    # Relatórios por seed e de todas juntas
    total = empty_state()
    print(f"\n{'seed':<6} {'arquivos':>8} {'instr.':>8} {'pares':>14} {'triplas':>18} "
          f"{'retiradas':>16} {'sem log':>10} {'com log':>10}")
    plain_all, log_all, retired_all = {}, {}, {}
    for seed in sorted(seed_files):
        state_path = out_root / seed / "state.json"
        if not state_path.is_file():
            continue
        state = load_state(state_path)
        hit, ref, n, p, t = write_report(state, out_root / seed, groups, reference, exclude,
                                         f"Seed {seed}")
        plain = state.get("plain_seconds", {})
        logged = state.get("log_seconds", {})
        plain_all.update({f"{seed}/{k}": v for k, v in plain.items()})
        retired_all.update({f"{seed}/{k}": v for k, v in state.get("retired", {}).items()})
        log_all.update({f"{seed}/{k}": v for k, v in logged.items()})
        print(f"{seed:<6} {len(state['files']):>8} {hit:>4}/{ref:<3} {p:>6}/{n * n:<7} "
              f"{t:>8}/{n ** 3:<9} {sum(state.get('retired', {}).values()):>16} "
              f"{(f'{sum(plain.values()):.1f}s' if plain else '-'):>10} "
              f"{(f'{sum(logged.values()):.1f}s' if logged else '-'):>10}")
        merge(total, {"file": seed, **{k: state[k] for k in ("counts", "configs", "pairs", "triples")}})
    if total["files"]:
        total_files = sum(len(load_state(out_root / s / "state.json")["files"]) for s in total["files"])
        total["files"] = [f"{s}" for s in range(total_files)]
        total["plain_seconds"], total["log_seconds"] = plain_all, log_all
        total["retired"] = retired_all
        hit, ref, n, p, t = write_report(total, out_root / "all", groups, reference, exclude,
                                         "Todas as seeds")
        print(f"{'todas':<6} {total_files:>8} {hit:>4}/{ref:<3} {p:>6}/{n * n:<7} {t:>8}/{n ** 3:<9} "
              f"{sum(retired_all.values()):>16} "
              f"{(f'{sum(plain_all.values()):.1f}s' if plain_all else '-'):>10} "
              f"{(f'{sum(log_all.values()):.1f}s' if log_all else '-'):>10}")
        print("  sem log = soma do tempo de cada arquivo no Spike normal (um por vez); "
              "com log = soma das execuções de cobertura (em paralelo)")
        print(f"\nRelatórios em {out_root}/<seed>/report.txt e {out_root}/all/report.txt")


if __name__ == "__main__":
    signal.signal(signal.SIGTERM, lambda *_: sys.exit(143))
    main()
