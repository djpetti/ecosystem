{
  'targets': [
    {
      'target_name': 'swig_automata',
      'type': 'shared_library',
      'cflags': [
        '-fPIC',
        '-I"/usr/include/python2.7"',
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
          'action': ['swig', '-python', '-c++', '<@(_inputs)'],
          'action': ['./swig_transfer.sh', '<(DEPTH)'],
        },
      ],
    },
  ],
}
