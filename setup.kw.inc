<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kw-setup`.
 */

/**
 * Implements drush_COMMAND_init() for `drush kw-setup`.
 */
function drush_kw_setup_init() {
  kraftwagen_context_init_kraftwagen_root();
}

/**
 * Implements drush_COMMAND for `drush kw-setup`.
 */
function drush_kraftwagen_kw_setup() {
  // Find out where the Kraftwagen root is.
  $root = kraftwagen_context_get_option('root-path');

  // Determine config directory.
  if (!($dir_cnf = kraftwagen_context_get_option('cnf-dir'))) {
    return drush_set_error(dt('No cnf dir name set.'));
  }

  // Make sure the config directory exists.
  if (!is_dir($root . DIRECTORY_SEPARATOR . $dir_cnf)) {
    drush_mkdir($root . DIRECTORY_SEPARATOR . $dir_cnf);
    drush_log(dt('Config directory !dir created', array('!dir' => $root . DIRECTORY_SEPARATOR . $dir_cnf)), 'success');
  }
  else {
    drush_log(dt('Config directory !dir already exists', array('!dir' => $root . DIRECTORY_SEPARATOR . $dir_cnf)), 'warning');
  }

  // Determine path to config directory in the source.
  if (!($dir_src = kraftwagen_context_get_option('src-dir'))) {
    return drush_set_error(dt('No src dir name set.'));
  }
  if (!($dir_src_cnf = kraftwagen_context_get_option('src-cnf-dir'))) {
    return drush_set_error(dt('No src cnf dir name set.'));
  }

  // Stop if we could not find the config in the source.
  if (!is_dir($root . DIRECTORY_SEPARATOR . $dir_src . DIRECTORY_SEPARATOR . $dir_src_cnf)) {
    return drush_set_error(dt('Config directory in source !dir does not exist.', array('!dir' => $root . DIRECTORY_SEPARATOR . $dir_src . DIRECTORY_SEPARATOR . $dir_src_cnf)));
  }

  if (!($dir_originals = kraftwagen_context_get_option('environment-originals-dir'))) {
    return drush_set_error(dt('No environment originals directory set.'));
  }

  if (!is_dir($root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $dir_originals)) {
    drush_mkdir($root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $dir_originals);
  }

  return kraftwagen_setup(
    $root . DIRECTORY_SEPARATOR . $dir_cnf, 
    $root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $dir_originals,
    $root . DIRECTORY_SEPARATOR . $dir_src . DIRECTORY_SEPARATOR . $dir_src_cnf
  );
}

function kraftwagen_setup($cnf_dir, $originals_dir, $src_cnf_dir) {
  _kraftwagen_setup($cnf_dir, $originals_dir, $src_cnf_dir, '');
}

function _kraftwagen_setup($cnf_dir, $originals_dir, $src_cnf_dir, $subdir) {
  if (!($handle = opendir($src_cnf_dir . $subdir))) {
    return;
  }

  while (($name = readdir($handle)) !== FALSE) {
    if (is_link($src_cnf_dir . $subdir . DIRECTORY_SEPARATOR . $name)) {
      // something?
    }
    elseif (is_dir($src_cnf_dir . $subdir . DIRECTORY_SEPARATOR . $name)) {
      // Make sure the directory exists in cnf/.
      if (!is_dir($cnf_dir . $subdir . DIRECTORY_SEPARATOR . $name)) {
        drush_mkdir($cnf_dir . $subdir . DIRECTORY_SEPARATOR . $name);
      }
      // Make sure the directory exists in cnf/.originals/.
      if (!is_dir($originals_dir . $subdir . DIRECTORY_SEPARATOR . $name)) {
        drush_mkdir($originals_dir . $subdir . DIRECTORY_SEPARATOR . $name);
      }

      // Recursively walk into the directories.
      _kraftwagen_setup($cnf_dir, $originals_dir, $src_cnf_dir, $subdir . DIRECTORY_SEPARATOR . $name);
    }
    else {
      _kraftwagen_setup_file($cnf_dir, $originals_dir, $src_cnf_dir, $subdir . DIRECTORY_SEPARATOR . $name);
    }
  }

  closedir($handle);
}

function _kraftwagen_setup_file($cnf_dir, $originals_dir, $src_cnf_dir, $file) {
  // If the file is not present, simply copy it.
  if (!file_exists($cnf_dir . $file)) {
    _kraftwagen_setup_file_from_template($cnf_dir, $originals_dir, $src_cnf_dir, $file);
    return;
  }

  // If the file is present, but we do not have a original, we assume the file
  // is changed.
  if (!file_exists($originals_dir . $file)) {
    $changed = TRUE;
  } else {
    // File is changed if the MD5 sums of the files in the src/cnf and 
    // cnf/.orginals differ.
    $changed = md5_file($src_cnf_dir . $file) != md5_file($originals_dir . $file);
  }

  if (!$changed) {
    return;
  }

  _kraftwagen_setup_file_from_template($cnf_dir, $originals_dir, $src_cnf_dir, $file);
}

function _kraftwagen_setup_file_from_template($cnf_dir, $originals_dir, $src_cnf_dir, $file) {
  @copy($src_cnf_dir . $file, $originals_dir . $file);
  
  $contents = file_get_contents($src_cnf_dir . $file);
  $new_contents = _kraftwagen_setup_template($cnf_dir, $contents);

  file_put_contents($cnf_dir . $file, $new_contents);
}

function _kraftwagen_setup_template($cnf_dir, $contents) {
  $settings = _kraftwagen_setup_get_settings($cnf_dir);

  $result = _kraftwagen_setup_template_execute($settings, $contents);
  
  _kraftwagen_setup_set_settings($cnf_dir, $settings);

  return $result;
}

function _kraftwagen_setup_template_execute(&$settings, $contents) {
  _kraftwagen_setup_template_execute_replace_callback(NULL, $settings);

  $result = preg_replace_callback('/\%{([^\}]+)\}/', '_kraftwagen_setup_template_execute_replace_callback', $contents);

  $s = _kraftwagen_setup_template_execute_replace_callback(NULL, NULL);
  foreach ($s as $key => $value) {
    $settings[$key] = $value;
  }

  return $result;
}

function _kraftwagen_setup_template_execute_replace_callback($matches = NULL, $settings = NULL) {
  static $cache;

  if (!isset($matches)) {
    if (isset($settings)) {
      $cache = $settings;
    }
    return $cache;
  }

  
}

function _kraftwagen_setup_get_settings($cnf_dir, $settings = NULL) {
  static $cache = array();

  if (isset($settings)) {
    $cache[$cnf_dir] = $settings;
  }

  if (!isset($cache[$cnf_dir])) {
    $cache[$cnf_dir] = array();
  }

  return $cache[$cnf_dir];
}

function _kraftwagen_setup_set_settings($cnf_dir, $settings) {
  _kraftwagen_setup_get_settings($cnf_dir, $settings);
}
