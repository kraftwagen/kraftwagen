<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kw-setup-settings`.
 */

/**
 * Implements drush_COMMAND_init() for `drush kw-setup-settings`.
 */
function drush_kw_setup_settings_init() {
  kraftwagen_context_init_kraftwagen_root();
}

/**
 * Implements drush_COMMAND for `drush kw-setup-settings`.
 */
function drush_kraftwagen_kw_setup_settings() {
  // Find out where the Kraftwagen root is.
  $root = kraftwagen_context_get_option('root-path');

  // Determine config directory.
  if (!($dir_cnf = kraftwagen_context_get_option('cnf-dir'))) {
    return drush_set_error(dt('No cnf dir name set.'));
  }

  if (!is_dir($root . DIRECTORY_SEPARATOR . $dir_cnf)) {
    return drush_set_error(dt('Could not use environment in !dir. Setup environment first.', array('!dir' => $root . DIRECTORY_SEPARATOR . $dir_cnf)));
  }

  $file_keys = array('settings-file', 'settings-local-file', 'kraftwagenrc-file');
  $file_names = array();
  foreach ($file_keys as $key) {
    if ($key == 'kraftwagenrc-file') {
      $name = KRAFTWAGEN_RC;
    } 
    elseif (!($name = kraftwagen_context_get_option($key))) {
      return drush_set_error(dt('No !key name set.', array('!key' => str_replace('-', ' ', $key))));
    }
    $file_names[$key] = $name;
  }

  if (!($dir_src = kraftwagen_context_get_option('src-dir'))) {
    return drush_set_error(dt('No src dir name set.'));
  }
  if (!($dir_src_cnf = kraftwagen_context_get_option('src-cnf-dir'))) {
    return drush_set_error(dt('No src cnf dir name set.'));
  }

  foreach ($file_names as $key => $file_name) {
    if (!file_exists($root . DIRECTORY_SEPARATOR . $dir_src . DIRECTORY_SEPARATOR . $dir_src_cnf . DIRECTORY_SEPARATOR . $file_name)) {
      drush_log(dt('Could not find !key at !path.', array('!key' => str_replace('-', ' ', $key), '!path' => $root . DIRECTORY_SEPARATOR . $dir_src . DIRECTORY_SEPARATOR . $dir_src_cnf . DIRECTORY_SEPARATOR . $file_name)), 'warning');
      continue;
    }

    if (file_exists($root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $file_name)) {
      drush_log(dt('!key at !path already exists.', array('!key' => str_replace('-', ' ', $key), '!path' => $root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $file_name)), 'warning');
      continue;
    }

    @copy($root . DIRECTORY_SEPARATOR . $dir_src . DIRECTORY_SEPARATOR . $dir_src_cnf . DIRECTORY_SEPARATOR . $file_name, $root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $file_name);
    drush_log(dt('Copied !key from !src to !dest', array('!key' => str_replace('-', ' ', $key), '!src' => $root . DIRECTORY_SEPARATOR . $dir_src . DIRECTORY_SEPARATOR . $dir_src_cnf . DIRECTORY_SEPARATOR . $file_name, '!dest' => $root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $file_name)), 'success');
  }

  drush_log(dt('You\'ll want to alter !path to fit your configuration', array('!path' => $root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . 'settings.local.php')), 'ok');
}
