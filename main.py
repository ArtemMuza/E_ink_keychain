def split_file(input_file):
    try:
        with open(input_file, 'r') as file:
            lines = file.readlines()

            for i, line in enumerate(lines):
                output_file_name = f"{i + 1}.txt"
                with open(output_file_name, 'w') as output_file:
                    output_file.write(line)

        print(f"Files are splitted. {i + 1} files created.")
    except FileNotFoundError:
        print(f"File {input_file} not found")
    except Exception as e:
        print(f"Error: {str(e)}")

input_file = input("Enter file name: ")
split_file(input_file)
