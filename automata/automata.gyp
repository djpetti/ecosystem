{
  'targets': [
    {
      'target_name': 'automata',
      'type': 'static_library',
      'sources': [
        'grid.cc',
        'movement_factor.cc',
        'organism.cc',
        'grid_object.cc',
      ],
    },
    {
      'target_name': 'automata_test',
      'type': 'executable',
      'dependencies': [
        'automata',
        '<(externals):gtest',
      ],
      'sources': [
        'automata_test.cc',
      ],
    },
  ],
}
