import os

def delete_txt_files():
    current_dir = os.getcwd()
    files_in_dir = os.listdir(current_dir)
    txt_files = [file for file in files_in_dir if file.endswith('.txt')]

    if not txt_files:
        print("No .txt files found in the directory.")
        return

    for txt_file in txt_files:
        file_path = os.path.join(current_dir, txt_file)
        try:
            os.remove(file_path)
            print(f"Deleted: {txt_file}")
        except OSError as e:
            print(f"Error deleting {txt_file}: {e}")

if __name__ == "__main__":
    delete_txt_files()
