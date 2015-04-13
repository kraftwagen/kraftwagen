<?php

/**
 * @file
 * This file provides helper functions for detection of actual environment
 *  and
 */

/**
 * Function to retrieve the actual environment from the environment file.
 */
function kraftwagen_context_environment() {
  // Find out where the Kraftwagen root is.
  $root = kraftwagen_context_get_option('root-path');

  // Determine config directory.
  if (!($dir_cnf = kraftwagen_context_get_option('cnf-dir'))) {
    return drush_set_error(dt('No cnf dir name set.'));
  }
  if (!($file_environment = kraftwagen_context_get_option('environment-file'))) {
    return drush_set_error(dt('No environment file set.'));
  }

  if (!file_exists($root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $file_environment)) {
    return drush_set_error(dt('No environment file found at !path.', array('!path' => $root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $file_environment)));
  }

  $environment = trim(file_get_contents($root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $file_environment));

  return $environment;
}