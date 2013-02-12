<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kw-activate-build`.
 */

/**
 * Implements drush_COMMAND_init() for `drush kw-activate-build`.
 */
function drush_kw_activate_build_init() {
  kraftwagen_context_init_kraftwagen_root();
}

/**
 * Implements drush_COMMAND() for `drush kw-activate-build`.
 */
function drush_kraftwagen_kw_activate_build($build) {
  // Find out where the Kraftwagen root is
  $root = kraftwagen_context_get_option('root-path');

  $dir_build = kraftwagen_context_get_option('build-dir');

  // If we have settings for the base build location (not in builds directory),
  // but we build in another place, check if we can manage some symlink to it.
  if (!$dir_build || $build == ($root . DIRECTORY_SEPARATOR . $dir_build)) {
    return drush_log(dt('Build at !path is already activated.', array('!path' => $build)), 'success');
  }

  // If base build is non-existant
  // @TODO: Find out if there is a better way then !@lstat() to check if a
  // path is not taken. lstat is probably a pretty IO intensive operation,
  // while the required information is actually very simple.
  if (@lstat($root . DIRECTORY_SEPARATOR . $dir_build)) {
    if (!is_link($root . DIRECTORY_SEPARATOR . $dir_build)) {
      // First try to archive the existing build
      require_once dirname(__FILE__) . '/includes/kraftwagen.archive.inc';
      if (is_dir($root . DIRECTORY_SEPARATOR . $dir_build)) {
        $archive_dest = kraftwagen_archive_directory($dir_build, $root . DIRECTORY_SEPARATOR . $dir_build);
      } else {
        $archive_dest = kraftwagen_archive_file($dir_build, $root . DIRECTORY_SEPARATOR . $dir_build);
      }
      if ($archive_dest) {
        drush_log(dt('Build archived to !dest.', array('!dest' => $archive_dest)), 'success');
      } else {
        return drush_set_error(dt('Could not archive build.'));
      }
    }

    drush_shell_exec('rm %s', $root . DIRECTORY_SEPARATOR . $dir_build);
  }

  symlink($build, $root . DIRECTORY_SEPARATOR . $dir_build);
  drush_log(dt('Symlink created from !link to !target.', array('!link' => $root . DIRECTORY_SEPARATOR . $dir_build, '!target' => $build)), 'success');
}

