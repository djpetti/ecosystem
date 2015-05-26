{
  'targets': [
    {
      'target_name': 'ecosystem_all',
      'type': 'none',
      'dependencies': [
        '<(DEPTH)/automata/swig/swig.gyp:*',
        '<(DEPTH)/automata/automata.gyp:automata_test',
        '<(DEPTH)/automata/metabolism/metabolism.gyp:plant_metabolism_test',
        '<(DEPTH)/automata/metabolism/metabolism.gyp:animal_metabolism_test',
      ],
    },
  ],
}
