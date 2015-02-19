{
  'targets': [
    {
      'target_name': 'automata_test',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/automata/automata.gyp:automata',
        '<(externals):gtest',
      ],
      'sources': [
        'automata_test.cc',
      ],
    },
  ],
}
