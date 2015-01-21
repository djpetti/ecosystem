{
  'targets': [
    {
      'target_name': 'grid_test',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/automata/automata.gyp:automata',
        '<(externals):gtest',
      ],
      'sources': [
        'grid_test.cc',
      ],
    },
  ],
}
