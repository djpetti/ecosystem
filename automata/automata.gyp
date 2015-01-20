{
  'targets': [
    {
      'target_name': 'libautomata',
      'type': 'static_library',
      'sources': [
        'grid.cc',
        'movement_factor.cc',
      ],
    },
    {
      'target_name': 'grid_test',
      'type': 'executable',
      'dependencies': [
        'libautomata',
        '<(externals):gtest',
      ],
      'sources': [
        'grid_test.cc',
      ],
    },
  ],
}
