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

$options['update-commands'] = array_fill_keys(
  array(
    'updatedb',
    'kraftwagen-dependencies',
    'features-revert-all',
    'kw-manifests',
  ),
  array('--yes')
);

$options['project-init-commands'] = array(
  'kraftwagen-environment-setup' => array(),
  'kraftwagen-environment-setup-settings' => array(),
);

$options['build-init-commands'] = array(
  array('kraftwagen-build-symlink' => array($options['settings-file'])),
  array('kraftwagen-build-symlink' => array($options['settings-local-file'])),
  array('kraftwagen-build-symlink' => array($options['files-dir'])),
);
