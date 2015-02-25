{
  'targets': [
    {
      'target_name': 'swig_automata',
      'type': 'shared_library',
      'cflags': [
        '-fPIC',
        '-I"/usr/include/python3.4"',
      ],
      'sources': [
        'automata_wrap.cxx',
      ],
      'dependencies': [
        '<(DEPTH)/automata/automata.gyp:automata',
      ],
      'actions': [
        {
          'action_name': 'swig',
          'inputs': [
            'automata.i',
          ],
          'outputs': [
            'automata_wrap.cxx',
            'automata.py',
          ],
          'action': ['swig', '-python', '-py3', '-modern', '-c++',
              '<@(_inputs)'],
        },
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
