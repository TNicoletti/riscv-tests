import sys
import struct
import os

def main():
    # Ensure the user provided a file name as a parameter
    if len(sys.argv) != 2:
        print("Usage: python mem_setter.py <output_filename>")
        sys.exit(1)

    filename = sys.argv[1]

    try:
        # Ask for the total number of integers to be written
        count_input = input("How many integers will be set? ")
        count = int(count_input, 0) # The '0' base allows both hex (0x...) and decimal inputs
    except ValueError:
        print("Error: Please enter a valid integer.")
        sys.exit(1)

    # Use 'r+b' (read/write binary) if the file exists so we don't truncate it.
    # Use 'w+b' (write/read binary) if it's a brand-new file.
    file_mode = 'r+b' if os.path.exists(filename) else 'w+b'

    try:
        with open(filename, file_mode) as f:
            
            # '<i' means: little-endian (<), 32-bit signed integer (i)
            # This overwrites the very first 4 bytes with the new count
            f.write(struct.pack('<i', count))
            
            for i in range(count):
                # Remember our position in the file before we read
                current_pos = f.tell()
                
                # Attempt to read the next 4 bytes to see if an integer already exists here
                existing_bytes = f.read(4)
                
                while True:
                    if len(existing_bytes) == 4:
                        # An integer exists here. Unpack and show it.
                        existing_val = struct.unpack('<i', existing_bytes)[0]
                        prompt = f"Value {i + 1} is {existing_val} (0x{existing_val:x}). Enter new value, or press Enter to keep: "
                        val_input = input(prompt)
                        
                        # If the user just presses Enter, keep the old value
                        if val_input.strip() == "":
                            val = existing_val
                            break
                    else:
                        # No existing integer here (we hit the end of the file)
                        val_input = input(f"Enter number {i + 1}: ")
                        if val_input.strip() == "":
                            print("Error: No existing value here. You must enter a number.")
                            continue
                            
                    try:
                        val = int(val_input, 0)
                        break
                    except ValueError:
                        print(f"Error: '{val_input}' is not a valid integer.")

                # Move the file pointer back to the start of this specific integer's slot
                f.seek(current_pos)
                
                try:
                    # Overwrite (or append) the new or kept value
                    f.write(struct.pack('<i', val))
                except struct.error:
                    print("Error: Number out of bounds for a 32-bit integer.")
                    sys.exit(1)

        print(f"\nSuccess! Wrote the count ({count}) and processed numbers in '{filename}'.")

    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    main()