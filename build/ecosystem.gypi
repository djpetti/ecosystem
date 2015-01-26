{
  'target_defaults': {
    'cflags': [
      '-std=c++11',
      '-Werror',
      '-Wall',
      '-fPIC',
      '-g',
      '-O0',
    ],
    'include_dirs': [
      '<(DEPTH)/',
    ],
  },
  'variables': {
    'externals': '<(DEPTH)/build/externals/externals.gyp',
  },
}
