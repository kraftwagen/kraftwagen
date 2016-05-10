<?php
/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kw-setup-symlink`.
 */

/**
 * Implements drush_COMMAND_init() for `drush kw-setup-symlink`.
 */
function drush_kw_setup_env_settings_init() {
  kraftwagen_context_init_kraftwagen_root();
}

/**
 * Implements drush_COMMAND for `drush kraftwagen-build-symlink`.
 *
 * @param string $build
 *   The location of the build where the symlink should created in.
 * @param string $env
 *   The name of environment
 */
function drush_kraftwagen_kw_setup_env_settings($build, $env = NULL) {
  // Find out where the Kraftwagen root is.
  $root = kraftwagen_context_get_option('root-path');

  if (!@lstat($build)) {
    return drush_set_error(dt('No build found at !path', array('!path' => $build)));
  }

  if (!($dir_drupal_config = kraftwagen_context_get_option('drupal-config-dir'))) {
    return drush_set_error(dt('No drupal config dir name set.'));
  }

  if (!($dir_cnf = kraftwagen_context_get_option('cnf-dir'))) {
    return drush_set_error(dt('No cnf dir name set.'));
  }

  if (!($env_settings_file = kraftwagen_context_get_option('environment-settings-file'))) {
    return drush_set_error(dt('No environment settings file name set.'));
  }

  if (!empty($env)) {
    if (!($file_environment = kraftwagen_context_get_option('environment-file'))) {
      _drush_kraftwagen_kw_setup_env_settings_remove_symlink($root, $build, $dir_drupal_config, $env_settings_file);
      return drush_set_error(dt('No environment file set.'));
    }

    if (!file_exists($root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $file_environment)) {
      _drush_kraftwagen_kw_setup_env_settings_remove_symlink($root, $build, $dir_drupal_config, $env_settings_file);
      return drush_set_error(dt('No environment file found at !path.', array('!path' => $root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $file_environment)));
    }

    $environment = trim(file_get_contents($root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $file_environment));
  }

  if (empty($setting_files = kraftwagen_context_get_option('environment-settings-files'))) {
    _drush_kraftwagen_kw_setup_env_settings_remove_symlink($root, $build, $dir_drupal_config, $env_settings_file);
    return drush_log(dt('No environment specific config found in kraftwagenrc.php'), 'warning');
  }

  if (empty($setting_files[$environment])) {
    _drush_kraftwagen_kw_setup_env_settings_remove_symlink($root, $build, $dir_drupal_config, $env_settings_file);
    return drush_log(dt('No environment specific config found for !environment', array('!environment', $environment)), 'warning');
  }


  $name = $setting_files[$environment];

  if (!@lstat($build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config)) {
    drush_mkdir($build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config);
    drush_log(dt('Drupal config directory !path created.', array('!path' => $build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config)), 'success');
  }

  if (!@lstat($root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $name)) {
    drush_log(dt('Could not find environment specific settings file source at !path', array('!path' => $root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $name)), 'warning');
    _drush_kraftwagen_kw_setup_env_settings_remove_symlink($root, $build, $dir_drupal_config, $env_settings_file);
    return;
  }

  require_once dirname(__FILE__) . '/includes/kraftwagen.fileutils.inc';
  if (file_exists($build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config . DIRECTORY_SEPARATOR . $env_settings_file)) {
    unlink($build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config . DIRECTORY_SEPARATOR . $env_settings_file);
  }
  kraftwagen_symlink($root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $name, $build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config . DIRECTORY_SEPARATOR . $env_settings_file);

  drush_log(dt('Symlinked envoirment settings file \'!name\' to build.', array('!name' => $name . (is_dir($root . DIRECTORY_SEPARATOR . $dir_cnf . DIRECTORY_SEPARATOR . $name) ? '/' : ''))), 'success');

}

function _drush_kraftwagen_kw_setup_env_settings_remove_symlink($root, $build, $dir_drupal_config, $env_settings_file) {
  if (file_exists($build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config . DIRECTORY_SEPARATOR . $env_settings_file)) {
    unlink($build . DIRECTORY_SEPARATOR . 'sites' . DIRECTORY_SEPARATOR . $dir_drupal_config . DIRECTORY_SEPARATOR . $env_settings_file);
    return drush_log(dt('Removed old symlink !env_settings_file from build', array('!env_settings_file' => $env_settings_file)), 'warning');
  }
}
