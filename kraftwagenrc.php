<?php

/**
 * @file
 * Kraftwagen settings defaults
 */

// directory names
$dirs = array(
  'src', 'tools', 'build', 'cnf', // default dirs for normal development
  'builds', // directory to keep track of all builds, for production and staging
  'archive', // directory to save, for example, old make files

  'src-tools', // tools directory in the src
  'src-cnf', // cnf directory in the src
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

  'environment-default' => 'production', // default environment is production for safety reasons
  'environment-file' => 'environment',

  'settings-file' => 'settings.php',
  'settings-local-file' => 'settings.local.php',
  'files-dir' => 'files',
  'drupal-config-dir' => 'default',
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

$options['build-commands'] = array(
  'kw-generate-makefile' => array('*make_file_location*'),
  'make' => array('*make_file_location*', '*target_dir*'),
  'kw-setup-symlinks' => array('*target_dir*'),
  'kw-activate-build' => array('*target_dir*'),
);

$options['update-commands'] = array(
  'kw-apply-module-dependencies' => array(), // make sure all required modules are enabled
  'updatedb' => array('--yes'), // run all hook_update_N implementation of enabled modules
  'features-revert-all' => array('--yes'), // make sure all feature definitions are applied 
  'kw-manifests' => array('--yes'), // run all manifests of all enabled modules
);

$options['setup-commands'] = array(
  'kw-setup-env',
  'kw-setup-settings',
  'kw-setup-upload-dir',
);

$options['setup-symlinks-commands'] = array(
  array('kw-setup-symlink' => array('*build*', $options['settings-file'])),
  array('kw-setup-symlink' => array('*build*', $options['settings-local-file'])),
  array('kw-setup-symlink' => array('*build*', $options['files-dir'])),
);
