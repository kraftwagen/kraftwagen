<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kw-init-database`.
 */

/**
 * Implements drush_COMMAND_init() for `drush kw-init-database`.
 */
function drush_kw_init_database_init() {
  kraftwagen_context_init_kraftwagen_root();
}

/**
 * Implements drush_COMMAND() for `drush kw-init-database`.
 */
function drush_kraftwagen_kw_init_database() {
  // Find out where the Kraftwagen root is.
  $root = kraftwagen_context_get_option('root-path');

  if (!($dir_src = kraftwagen_context_get_option('src-dir'))) {
    return drush_set_error(dt('No src dir name set.'));
  }

  if (!lstat($root . DIRECTORY_SEPARATOR . $dir_src)) {
    return drush_set_error(dt('Src dir not found at !path', array('!path' => $root . DIRECTORY_SEPARATOR . $dir_src)));
  }

  $files = drush_scan_directory($root . DIRECTORY_SEPARATOR . $dir_src, "/\.info$/", array('.', '..', '.git'), 0, FALSE);
  if (empty($files) || count($files) > 1) {
    return drush_set_error(dt('Could not determine profile name from src dir.'));
  }
  $files = array_values($files);
  $file = $files[0];

  $profile_name = $file->name;

  drush_invoke_process('@self', 'site-install', array($profile_name, '--yes'));
}