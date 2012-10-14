<?php

/**
 * @file
 * Kraftwagen settings defaults
 */

// directory names
$dirs = array(
  'src', 'tools', 'build', // default dirs for normal development
  'builds', // directory to keep track of all build, for production and staging
  'archive', // directory to save, for example, old make files

  'src-tools' // tools directory in the src
);
foreach ($dirs as $dir) { 
  $options["{$dir}-dir"] = current(array_reverse(explode('-', $dir)));
}

// other settings
$options += array(
  'make-file' => 'build.make',
  'make-file-tpl' => 'build.make.tpl',
  'date-pattern' => 'Ymd-His',
  'file-hashing-function' => 'sha1_file',
);

// default way to find the Kraftwagen root directory is checking for the 
// existance of the src-dir
$options['root-checks'] = array(
  array(
    'type' => 'require_directory',
    'parameters' => array(
      'dir' => '*src-dir*'
    )
  )
);
