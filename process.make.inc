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


/**
 * Implements make_download_DOWNLOAD_TYPE() for 'kraftwagen_symlink'.
 *
 *  projects[***MACHINE_NAME***][type] = "profile"
 *  projects[***MACHINE_NAME***][download][type] = "kraftwagen_symlink"
 *  projects[***MACHINE_NAME***][download][url] = "**SRC_DIR**  ($target)
*/
function make_download_kraftwagen_symlink() {
  $args = func_get_args();
  $name = array_shift($args);
  if (count($args) > 2) {
    $type = array_shift($args);
  }
  $target = array_shift($args);
  $link_name = array_shift($args);

  // Symlinks cannot be created on Windows OS.
  if (drush_is_windows()) {
    return drush_set_error('DRUSH_IS_WINDOWS', dt("Symlinks cannot be created on Windows platform."));
  }
  // Check if target is a directory. If so, delete it.
  if (is_dir($link_name)) {
    drush_delete_dir($link_name, TRUE);
  }
  // If it is a link, then unlink.
  elseif (is_link($link_name)) {
    unlink($link_name);
  }
  // Create the symlink.
  symlink($target['url'], $link_name);
  // Check if symlink was created correctly.
  if (is_link($link_name)) {
    drush_log(dt("Symlink '!link_name' to '!target' has been created.", array(
      '!link_name' => $link_name,
      '!target'    => $target['url'],
    )), 'success');
  }
  else {
    return drush_set_error('Error creating symlink', dt("Cannot create symlink '!link_name' to '!target'.", array(
      '!link_name' => $link_name,
      '!target'    => $target['url'],
    )), 'error');
  }
}
