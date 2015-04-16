{
  'targets': [
    {
      'target_name': 'ecosystem_all',
      'type': 'none',
      'dependencies': [
        '<(DEPTH)/automata/swig/swig.gyp:*',
        '<(DEPTH)/automata/metabolism/metabolism.gyp:plant_metabolism_test',
      ],
    },
  ],
}
