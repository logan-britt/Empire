import os
import sys
import shutil
from subprocess import run

def process_args(raw_args):
  processed_args = {}
  for i in range(len(raw_args)):
    arg = raw_args[i]
      
    if arg == '--build':
      processed_args['mode'] = 'build'
    elif arg == '--clean':
      processed_args['mode'] = 'clean'
    elif arg == '--debug':
      processed_args['debug'] = True
    elif arg == '--rebuild':
      processed_args['rebuild'] = True
    elif arg == '--verbose':
      processed_args['verbose'] = True

  if not 'debug' in processed_args:
    processed_args['debug'] = False
  if not 'rebuild' in processed_args:
    processed_args['rebuild'] = False
  if not 'verbose' in processed_args:
    processed_args['verbose'] = False
  return processed_args

def clean():
  shutil.rmtree('Empire')
  if 'vc140.pdb' in os.listdir():
    os.remove('vc140.pdb')

def execute(clang_flags, nvcc_flags):
  if 'win' in sys.platform:
    run([
      'clang++', 
      'source/merlin_help.cpp',
      'source/merlin.cpp',
      'source/merlin_draw.cpp',

      '-I/Vulkan/1.2.176.1/x86_64/include/',
      '-Ilibs/SDL2/include',
    
      '-LD:/Programming/Vulkan/x64/libs/'
      '-LEmpire'
      '-lvulkan-1',
      '-lsdl2d',

      *clang_flags,
      '--shared',
    
      '-o',
      f'Empire/libMerlin.dll'
    ])
  
    run([
      'clang++',
      'source/place.cpp',
      'source/place_building.cpp',
      'source/place_people.cpp',
      'source/place_resorces.cpp',

      *clang_flags,
      '--shared',

      '-o',
      f'Empire/libPlace.dll'
    ])

    run([
      'nvcc',
      'source/empire.cpp',
  
      '-I/Vulkan/1.2.176.1/x86_64/include/',
      '-Ilibs/SDL2/include',
      *nvcc_flags,
  
      '-lEmpire/libMerlin',
      '-lEmpire/libPlace',
      '-lEmpire/sdl2d',
  
      '-o',
      'Empire/empire.exe'
    ])

  else:
    run([
      'clang++', 
      'source/merlin_help.cpp',
      'source/merlin.cpp',
      'source/merlin_draw.cpp',

      'source/ui.cpp',

      '-I/Vulkan/1.2.176.1/x86_64/include/',
      '-Ilibs/SDL2/include',

      '-L/Vulkan/1.2.176.1/x86_64/lib',
      '-lvulkan',

      '-LEmpire/',
      '-lSDL2-2.0',

      *clang_flags,
      '--shared',
      '-fPIC',
    
      '-o',
      f'Empire/libMerlin.so'
    ])
  
    run([
      'clang++',
      'source/place.cpp',
      'source/place_building.cpp',
      'source/place_people.cpp',
      'source/place_resorces.cpp',

      *clang_flags,
      '--shared',
      '-fpic',

      '-o',
      f'Empire/libPlace.so'
    ])

    run([
      'nvcc',
      'source/empire.cpp',

      '-I/Vulkan/1.2.176.1/x86_64/include/',
      '-Ilibs/SDL2/include',
      *nvcc_flags,

      '-LEmpire/',
      '-lMerlin',
      '-lPlace',
      '-lSDL2-2.0',

      '-o',
      'Empire/empire'
    ])

def build(debug, rebuild, verbose):
  debug_flags_clang = ['-g', '-O0']
  debug_flags_nvcc = ['-G', '-g']

  clang_flags = []
  nvcc_flags = []
  if debug:
    for i in range(2):
      clang_flags.append(debug_flags_clang[i])
      nvcc_flags.append(debug_flags_nvcc[i])

  if verbose:
    clang_flags.append('-v')
    nvcc_flags.append('-v')

  if not rebuild:
    os.mkdir('Empire')
    os.mkdir('Empire/shaders')
    os.mkdir('Empire/textures')

    os.mkdir('sdl2-build')
    os.chdir('sdl2-build')
    run(['cmake', '-G', 'Ninja', '../libs/SDL2'])
    run('ninja')
    if 'win' in sys.platform:
      shutil.copy('sdl2d.lib', '../Empire/sdl2d.lib')
      shutil.copy('sdl2d.dll', '../Empire/sdl2d.dll')
      shutil.copy('sdl2d.exp', '../Empire/sdl2d.exp')
      if debug:
        pass

    else:
      shutil.copy('libSDL2-2.0.so', '../Empire/libSDL2-2.0.so')
      shutil.copy('libSDL2-2.0.so.0', '../Empire/libSDL2-2.0.so.0')
      shutil.copy('libSDL2-2.0.so.0.14.0', '../Empire/libSDL2-2.0.so.0.14.0')
      if debug:
        pass

    os.chdir('..')
    shutil.rmtree('sdl2-build')

  for file_name in os.listdir('shaders'):
    if '.vert' in file_name:
      new_name  = file_name.split('.')
      new_name = f'{new_name[0]}_vert.spv'
      run(['glslc', f'shaders/{file_name}', '-o', f'Empire/shaders/{new_name}'])

    elif '.frag' in file_name:
      new_name  = file_name.split('.')
      new_name = f'{new_name[0]}_frag.spv'
      run(['glslc', f'shaders/{file_name}', '-o', f'Empire/shaders/{new_name}'])

    else:
      print('Unrecognised shader found.')

  execute(clang_flags, nvcc_flags)

def main(arguments):
  args = process_args(arguments)

  if args['mode'] == 'build':
    build(args['debug'], args['rebuild'], args['verbose'])

  else:
    clean()

if __name__ == '__main__':
  main(sys.argv)