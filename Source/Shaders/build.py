import sys
import os

PROJECT_PATH = sys.argv[1]
SHADER_SOURCE_PATH = os.path.join(PROJECT_PATH, 'Source/Shaders')
SHADER_OUTPUT_PATH = os.path.join(PROJECT_PATH, 'SPIR-V')

def compile_shaders(source_path, output_path, extension) -> None:
	soruce_files = [f for f in os.listdir(source_path) if f.endswith(extension) and os.path.isfile(os.path.join(source_path, f))]
	for input_file in soruce_files:
		output_fullname = os.path.join(output_path, os.path.splitext(input_file)[0] + '.spv')
		input_fullname = os.path.join(source_path, input_file)
		if not os.path.exists(output_fullname) or os.path.getmtime(input_fullname) > os.path.getmtime(output_fullname):
			command = f'glslang -V -o {output_fullname} {input_fullname}'
			print(f'Compiling {input_file}...')
			if os.system(command) != 0:
				raise Exception(f"Cannot compile {input_file}.")

try:
	print('Compiling vertex shaders...')
	compile_shaders(os.path.join(SHADER_SOURCE_PATH, 'Vertex'), os.path.join(SHADER_OUTPUT_PATH, 'Vertex'), '.vert')
	print('Compiling fragment shaders...')
	compile_shaders(os.path.join(SHADER_SOURCE_PATH, 'Fragment'), os.path.join(SHADER_OUTPUT_PATH, 'Fragment'), '.frag')
	print('Compilation completed.')
except Exception as e:
	print(f'Compilation failed: {e}')
	sys.exit(1)