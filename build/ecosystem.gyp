{
  'targets': [
    {
      'target_name': 'ecosystem_all',
      'type': 'none',
      'dependencies': [
        '<(DEPTH)/automata/automata.gyp:*',
        '<(DEPTH)/automata/tests/tests.gyp:*',
        '<(DEPTH)/automata/swig/swig.gyp:*',
      ],
    },
  ],
}
