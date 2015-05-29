{
  'targets': [
    {
      'target_name': 'swig_automata',
      'type': 'shared_library',
      'cflags': [
        '-fPIC',
        '-I"/usr/include/python3.4"',
        '-Wno-unused-label', # Stupid swig.
      ],
      'sources': [
        'automata_wrap.cxx',
      ],
      'dependencies': [
        '<(DEPTH)/automata/automata.gyp:automata',
        '<(DEPTH)/automata/metabolism/metabolism.gyp:metabolism'
      ],
      'actions': [
        {
          'variables': {
            'libautomata_files': [
              # We include the .h files so the swig library gets rebuilt when
              # they get updated.
              '<(DEPTH)/automata/grid.cc',
              '<(DEPTH)/automata/grid.h',
              '<(DEPTH)/automata/grid_object.cc',
              '<(DEPTH)/automata/grid_object.h',
              '<(DEPTH)/automata/movement_factor.cc',
              '<(DEPTH)/automata/movement_factor.h',
              '<(DEPTH)/automata/organism.cc',
              '<(DEPTH)/automata/organism.h',
              '<(DEPTH)/automata/metabolism/plant_metabolism.cc',
              '<(DEPTH)/automata/metabolism/plant_metabolism.h',
              '<(DEPTH)/automata/metabolism/animal_metabolism.cc',
              '<(DEPTH)/automata/metabolism/animal_metabolism.h',
              '<(DEPTH)/automata/macros.h',
            ],
          },
          'action_name': 'swig',
          'inputs': [
            'automata.i',
            'metabolism.i',
            '<@(libautomata_files)',
          ],
          'outputs': [
            'automata_wrap.cxx',
            'automata.py',
          ],
          'action': ['swig', '-python', '-py3', '-modern', '-c++',
              'automata.i'],
        },
      ],
    },
    {
      'target_name': 'swig_transfer_proxy',
      'type': 'none',
      'dependencies': [
        'swig_automata',
      ],
      'actions': [
        {
          'action_name': 'swig_transfer',
          'inputs': [
            'automata.py',
            'automata_wrap.cxx',
          ],
          'outputs': [
            '<(DEPTH)/swig_modules/automata.py',
            '<(DEPTH)/swig_modules/_automata.so',
          ],
          'action': ['./swig_transfer.sh', '<(DEPTH)'],
        },
      ],
    },
  ],
}
