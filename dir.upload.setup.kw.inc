<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kw-setup-upload-dir`.
 */

/**
 * Implements drush_COMMAND_init() for `drush kw-setup-upload-dir`.
 */
function drush_kw_setup_upload_dir_init() {
  kraftwagen_context_init_kraftwagen_root();
}

/**
 * Implements drush_COMMAND for `drush kw-setup-upload-dir`.
 */
function drush_kraftwagen_kw_setup_upload_dir() {
  // Find out where the Kraftwagen root is.
  $root = kraftwagen_context_get_option('root-path');

  // Determine config directory.
  if (!($dir_cnf = kraftwagen_context_get_option('cnf-dir'))) {
    return drush_set_error(dt('No cnf dir name set.')); 
  }

  if (!($dir_files = kraftwagen_context_get_option('files-dir'))) {
    return drush_set_error(dt('No files dir name set.')); 
  }

  if (is_dir($root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $dir_files)) {
    drush_log(dt('Files dir at !path already exists', array('!path' => $root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $dir_files)), 'warning');
  } else {
    drush_mkdir($root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $dir_files);
    drush_log(dt('Files dir at !path created', array('!path' => $root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $dir_files)), 'success');
  }
}