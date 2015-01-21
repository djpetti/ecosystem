{
  'target_defaults': {
    'cflags': [
      '-std=c++11',
      '-Werror',
      '-Wall',
      '-fPIC',
    ],
    'include_dirs': [
      '<(DEPTH)/',
    ],
  },
  'variables': {
    'externals': '<(DEPTH)/build/externals/externals.gyp',
  },
}
