<?php

/**
 * @file
 * Kraftwagen settings defaults
 */

// Directory names:
$dirs = array(
  // Default dirs for normal development;
  'src', 'tools', 'build', 'cnf',
  // Directory to keep track of all builds, for production and staging:
  'builds',
  // Tools directory in the src:
  'src-tools',
  // Cnf directory in the src:
  'src-cnf',
);
foreach ($dirs as $dir) {
  $options["{$dir}-dir"] = current(array_reverse(explode('-', $dir)));
}

// Other settings:
$options += array(
  'make-file' => 'build.make',
  'make-file-tpl' => 'build.make.tpl',
  'date-pattern' => 'Ymd-His',
  'file-hashing-function' => 'sha1_file',

  // Default environment is production for safety reasons.
  'environment-default' => 'production',
  'environment-file' => 'environment',

  'settings-file' => 'settings.php',
  'settings-local-file' => 'settings.local.php',
  'files-dir' => 'files',
  'drupal-config-dir' => 'default',
  'use-relative-symlinks' => TRUE,
);

// Default way to find the Kraftwagen root directory is checking for the
// existance of the src-dir.
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
  'make' => array('*make_file_location*', '*target_dir*','--concurrency=1'),
  'kw-setup-symlinks' => array('*target_dir*'),
  'kw-activate-build' => array('*target_dir*'),
);

$options['update-commands'] = array(
  // Make sure all required modules are enabled.
  'kw-apply-module-dependencies' => array('*environment*'),
  // Run all hook_update_N implementation of enabled modules.
  'updatedb' => array('--yes'),
  // Make sure all feature definitions are applied.
  'features-revert-all' => array('--yes'),
  // Run all manifests of all enabled modules.
  'kw-manifests' => array('*environment*', '--yes'),
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
