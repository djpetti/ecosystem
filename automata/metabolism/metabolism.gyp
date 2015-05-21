{
  'targets': [
    {
      'target_name': 'metabolism',
      'type': 'static_library',
      'sources': [
        'plant_metabolism.cc',
        'animal_metabolism.cc',
      ],
    },
    {
      'target_name': 'plant_metabolism_test',
      'type': 'executable',
      'sources': [
        'plant_metabolism_test.cc',
      ],
      'dependencies': [
        'metabolism',
        '<(externals):gtest',
      ],
    },
    {
      'target_name': 'animal_metabolism_test',
      'type': 'executable',
      'sources': [
        'animal_metabolism_test.cc',
      ],
      'dependencies': [
        'metabolism',
        '<(externals):gtest',
      ],
    },
  ],
}
