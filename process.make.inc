<?php

/**
 * @file
 * This file provides an extension to `drush make-process`, a subcommand
 * of `drush make`.
 */

/**
 * Implements make_download_DOWNLOAD_TYPE() for 'kraftwagen_directory'.
 * This allows building directly from the filesystem, instead of from a
 * repository.
 */
function make_download_kraftwagen_directory() {
  $args = func_get_args();
  $name = array_shift($args);
  if (count($args) > 2) {
    $type = array_shift($args);
  }
  $download = array_shift($args);
  $download_location = array_shift($args);

  kraftwagen_context_init_kraftwagen_root();
  $env = kraftwagen_context_get_option('environment');

  // Better local development workflow.
  if ($env == 'local') {
    rmdir($download_location);

    $old_cwd = getcwd();
    chdir(dirname($download_location));
    $result = symlink($download['url'], $name);
    chdir($old_cwd);

    drush_log(dt('@project linked from @url.', array('@project' => $name, '@url' => $download['url'])), 'ok');
  }
  else {
    $result = drush_copy_dir($download['url'], $download_location, FILE_EXISTS_OVERWRITE);
    drush_log(dt('@project copied from @url.', array('@project' => $name, '@url' => $download['url'])), 'ok');
  }
  return $result ? $download_location : FALSE;
}