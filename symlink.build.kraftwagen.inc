<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kraftwagen-build-symlink`.
 */

/**
 * Implements drush_COMMAND_init() for `drush kraftwagen-build`.
 */
function drush_kraftwagen_build_symlink_init() {
  kraftwagen_context_init_kraftwagen_root();
}

/**
 * Implements drush_COMMAND for `drush kraftwagen-build-symlink`.
 */
function drush_kraftwagen_build_symlink($name) {
  // find out where the Kraftwagen root is
  $root = kraftwagen_context_get_option('root-path');

  if (!($dir_build = kraftwagen_context_get_option('build-dir'))) {
    return drush_set_error(dt('No build dir name set.')); 
  }
  
  if (!@lstat($root . DIRECTORY_SEPARATOR . $dir_build)) {
    return drush_set_error(dt('No build found at !path', array('!path' => $root . DIRECTORY_SEPARATOR . $dir_build)));
  }

  if (!($dir_drupal_config = kraftwagen_context_get_option('drupal-config-dir'))) {
    return drush_set_error(dt('No drupal config dir name set.')); 
  }

  if (!@lstat($root . DIRECTORY_SEPARATOR . $dir_build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config)) {
    drush_mkdir($root . DIRECTORY_SEPARATOR . $dir_build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config);
    drush_log(dt('Drupal config directory !path created.', array('!path' => $root . DIRECTORY_SEPARATOR . $dir_build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config)), 'success');
  }

  if (@lstat($root . DIRECTORY_SEPARATOR . $dir_build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config . DIRECTORY_SEPARATOR . $name)) {
    drush_log(dt('Symlink target !path already exists.', array('!path' => $root . DIRECTORY_SEPARATOR . $dir_build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config . DIRECTORY_SEPARATOR . $name)), 'warning');
    return;
  }

  if (!($dir_cnf = kraftwagen_context_get_option('cnf-dir'))) {
    return drush_set_error(dt('No cnf dir name set.')); 
  }

  if (!@lstat($root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $name)) {
    return drush_set_error(dt('Could not find symlink source at !path', array('!path' => $root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $name)));
  }

  symlink($root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $name, $root . DIRECTORY_SEPARATOR . $dir_build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config . DIRECTORY_SEPARATOR . $name);
  drush_log(dt('Symlink for !name created from !src to !dest', array('!name' => $name , '!src' => $root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $name, '!dest' => $root . DIRECTORY_SEPARATOR . $dir_build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config . DIRECTORY_SEPARATOR . $name)), 'success');
}
