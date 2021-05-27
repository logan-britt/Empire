import os
import sys
import shutil
from subprocess import run

linux_exts = {'shared':'.so'}
window_exts = {'shared':'.dll'}
if 'win' in sys.platform:
  exensions = window_exts
else:
  exensions = linux_exts

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
      processed_args['mode'] = 'rebuild'

  if not 'debug' in processed_args:
    processed_args['debug'] = False
  return processed_args

def clean():
  shutil.rmtree('Empire')
  if 'vc140.pdb' in os.listdir():
    os.remove('vc140.pdb')

def build(debug):
  debug_flags_clang = ['-g', '-O0']
  debug_flags_nvcc = ['-G', '-g']

  clang_flags = []
  nvcc_flags = []
  if debug:
    for i in range(2):
      clang_flags.append(debug_flags_clang[i])
      nvcc_flags.append(debug_flags_nvcc[i])

  os.mkdir('sdl2-build')
  os.chdir('sdl2-build')
  run(['cmake', '../libs/SDL2'])
  run('ninja')
  shutil.copy('sdl2d.lib', '../Empire/sdl2d.lib')
  shutil.copy('sdl2d.dll', '../Empire/sdl2d.dll')
  shutil.copy('sdl2d.exp', '../Empire/sdl2d.exp')
  if debug:
    shutil.copy('sdl2d.pdb', '../Empire/sdl2d.pdb')
  os.chdir('..')
  shutil.rmtree('sdl2-build')

  run([
    'clang++', 
    'source/merlin_help.cpp',
    'source/merlin.cpp',
    'source/merlin_draw.cpp',
    
    '-ID:/Programming/Vulkan/1.2.162.1/Include',
    '-Ilibs/SDL2/include',
    
    '-LD:/Programming/Vulkan/1.2.162.1/Lib',
    '-lvulkan-1',
    '-lEmpire/sdl2d',

    *clang_flags,
    '--shared',
    
    '-o',
    f'Empire/libMerlin{exensions["shared"]}'
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
    f'Empire/libPlace{exensions["shared"]}'
  ])

  run([
    'nvcc',
    'source/empire.cpp',

    '-ID:/Programming/Vulkan/1.2.162.1/Include',
    '-Ilibs/SDL2/include',
    *nvcc_flags,

    '-LEmpire',
    '-llibMerlin',
    '-llibPlace',

    '-lEmpire/sdl2d',

    '-o',
    'Empire/empire'
  ])

def rebuild(debug):
  pass

def main(arguments):
  args = process_args(arguments)

  if args['mode'] == 'build':
    os.mkdir('Empire')
    os.mkdir('Empire/shaders')
    os.mkdir('Empire/textures')
    
    if args['debug']:
      build(True)
    else:
      build(False)

  elif args['mode'] == 'rebuild':
    if args['debug']:
      rebuild(True)
    else:
      rebuild(False)

  else:
    clean()

if __name__ == '__main__':
  main(sys.argv)