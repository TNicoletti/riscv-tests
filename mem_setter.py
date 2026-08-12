import sys
import struct
import json
import os

def number_entries_precalc(variables):
    total_elements = 0
    for var in variables:
        if var.get("type") == "bool_array":
            val = var.get("value", [])
            if isinstance(val, list):
                total_elements += len(val)
        else:
            total_elements += 1
            
    print(f"Writting {total_elements} elements")
    return total_elements

def main():
    # Verifica se o usuário passou o arquivo JSON como parâmetro
    if len(sys.argv) != 2:
        print("Uso: python mem_setter.py <arquivo_config.json>")
        sys.exit(1)

    json_filepath = sys.argv[1]

    # Carrega o JSON
    try:
        with open(json_filepath, 'r', encoding='utf-8') as f:
            config = json.load(f)
    except Exception as e:
        print(f"Erro ao ler o arquivo JSON: {e}")
        sys.exit(1)

    
    json_dir = os.path.dirname(os.path.abspath(json_filepath))
    filename = os.path.join(json_dir, "params.mem")

    variables = config.get("variables", [])

    total_elements = number_entries_precalc(variables)

    # 'r+b' (read/write binary) se existir, 'w+b' se for um arquivo novo
    file_mode = 'r+b' if os.path.exists(filename) else 'w+b'

    try:
        with open(filename, file_mode) as f:
            # Garante que estamos escrevendo do início (ou sobrescrevendo sequencialmente)
            f.seek(0)

            f.write(struct.pack('<i', total_elements))
            
            for var in variables:
                name = var.get("name")
                v_type = var.get("type")
                val = var.get("value")

                if v_type == "int":
                    try:
                        # '<i': little-endian, inteiro com sinal de 32 bits (4 bytes)
                        f.write(struct.pack('<i', int(val)))
                        print(f"[{name}] Written integer: {val}")
                    except struct.error:
                        print(f"Error: Value '{val}' out of limits for 32bit integer.")
                        sys.exit(1)
                
                elif v_type == "bool":
                    # '<B': little-endian, unsigned char (1 byte)
                    b_val = 1 if val else 0
                    f.write(struct.pack('<i', int(b_val)))
                    #f.write(struct.pack('<B', b_val))

                    print(f"[{name}] Written bool: {b_val}")

                elif v_type == "bool_array":
                    if not isinstance(val, list):
                        print(f"Error: variable '{name}' should be a list.")
                        continue
                    
                    #packed_data = bytearray()
                    element_names = []
                    
                    # Itera sobre os elementos do array
                    for item in val:
                        # Se o item for um objeto nomeado (dicionário)
                        if isinstance(item, dict):
                            item_val = item.get("value", False)
                            item_name = item.get("name", "unnamed")
                        # Se for apenas um booleano puro (para manter compatibilidade)
                        else:
                            item_val = item
                            item_name = "unnamed"

                        b_val = 1 if item_val else 0
                        f.write(struct.pack('<i', b_val))
                        #packed_data.append(b_val)
                        element_names.append(item_name)
                    
                    #f.write(packed_data)
                    print(f"[{name}] Written bool_array with {len(val)} elements. ({', '.join(element_names)})")

                else:
                    print(f"Error: type '{v_type}' not recognized as a variable type '{name}'.")

        print(f"\nSuccess!")

    except Exception as e:
        print(f"Record error ocorred: {e}")

if __name__ == "__main__":
    main()