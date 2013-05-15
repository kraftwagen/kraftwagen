<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kw-setup-symlink`.
 */

/**
 * Implements drush_COMMAND_init() for `drush kw-setup-symlink`.
 */
function drush_kw_setup_symlink_init() {
  kraftwagen_context_init_kraftwagen_root();
}

/**
 * Implements drush_COMMAND for `drush kraftwagen-build-symlink`.
 *
 * @param string $build
 *   The location of the build where the symlink should created in.
 * @param string $name
 *   The name of the symlink to be created.
 */
function drush_kraftwagen_kw_setup_symlink($build, $name) {
  // Find out where the Kraftwagen root is.
  $root = kraftwagen_context_get_option('root-path');

  if (!@lstat($build)) {
    return drush_set_error(dt('No build found at !path', array('!path' => $build)));
  }

  if (!($dir_drupal_config = kraftwagen_context_get_option('drupal-config-dir'))) {
    return drush_set_error(dt('No drupal config dir name set.'));
  }

  if (!@lstat($build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config)) {
    drush_mkdir($build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config);
    drush_log(dt('Drupal config directory !path created.', array('!path' => $build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config)), 'success');
  }

  if (@lstat($build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config . DIRECTORY_SEPARATOR . $name)) {
    drush_log(dt('Symlink target !path already exists.', array('!path' => $build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config . DIRECTORY_SEPARATOR . $name)), 'warning');
    return;
  }

  if (!($dir_cnf = kraftwagen_context_get_option('cnf-dir'))) {
    return drush_set_error(dt('No cnf dir name set.'));
  }

  if (!@lstat($root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $name)) {
    return drush_set_error(dt('Could not find symlink source at !path', array('!path' => $root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $name)));
  }

  symlink($root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $name, $build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config . DIRECTORY_SEPARATOR . $name);
  drush_log(dt('Symlinked \'!name\' to build.', array('!name' => $name . (is_dir($root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $name) ? '/' : ''))), 'success');
}
