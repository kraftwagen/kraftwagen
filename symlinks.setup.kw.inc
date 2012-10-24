<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kw-setup-symlinks`.
 */

/**
 * Implements drush_COMMAND_init() for `drush kw-setup-symlinks`.
 */
function drush_kw_setup_symlink_init() {
  kraftwagen_context_init_kraftwagen_root();
}

/**
 * Implements drush_COMMAND for `drush kw-setup-symlinks`.
 */
function drush_kraftwagen_kw_setup_symlinks($build = null) {
  // find out where the Kraftwagen root is
  $root = kraftwagen_context_get_option('root-path');

  if (!isset($build)) {
    if (!($dir_build = kraftwagen_context_get_option('build-dir'))) {
      return drush_set_error(dt('No build dir name set.')); 
    }

    $build = $root . DIRECTORY_SEPARATOR . $dir_build;
  }

  if (!@lstat($build)) {
    return drush_set_error(dt('No build found at !path', array('!path' => $build)));
  }

  return kraftwagen_commands_sequence_run('setup-symlinks-commands', array('*build*' => $build));
}