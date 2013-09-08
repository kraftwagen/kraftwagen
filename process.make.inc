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

  $result = drush_copy_dir($download['url'], $download_location, FILE_EXISTS_OVERWRITE);
  drush_log(dt('@project copied from @url.', array('@project' => $name, '@url' => $download['url'])), 'ok');
  return $result ? $download_location : FALSE;
}
