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
 *
 * @param string $build
 *   The location of the build to be activated.
 */
function drush_kraftwagen_kw_activate_build($build = NULL) {
  // Find out where the Kraftwagen root is.
  $root = kraftwagen_context_get_option('root-path');

  $dir_build = kraftwagen_context_get_option('build-dir');
  $path_build = $root . DIRECTORY_SEPARATOR . $dir_build;

  $dir_builds = kraftwagen_context_get_option('builds-dir');
  $path_builds = $root . DIRECTORY_SEPARATOR . $dir_builds;

  if (empty($build)) {
    // Get dir listing and remove current (.) and parent (..) dir.
    $dir_list = scandir($path_builds);
    unset($dir_list[0], $dir_list[1]);

    // Build list of available builds.
    drush_print("Choose build:\n");

    $builds = array();
    $count = 1;
    $active_build = array_pop(explode(DIRECTORY_SEPARATOR, realpath($path_build)));
    foreach ($dir_list as $filename) {
      if (is_dir($path_builds . DIRECTORY_SEPARATOR . $filename)) {
        if (preg_match("/[0-9]{8}-[0-9]{6}/", $filename)) {
          drush_print(" " . $count . ") " . $filename . ($active_build == $filename ? " (current)" : ""));
          $builds[] = $filename;
          $count++;
        }
      }
    }
    drush_print("");

    $selected_build = drush_prompt(dt('Activate build'), NULL, TRUE);
    if (is_numeric($selected_build) && isset($builds[$selected_build - 1])) {
      $build = $path_builds . DIRECTORY_SEPARATOR . $builds[$selected_build - 1];
    }
    else {
      drush_log('Invalid selection', 'error');
      return FALSE;
    }
  }
  elseif (!empty($build) && is_string($build) && preg_match("/^[0-9]{8}-[0-9]{6}$/", $build)) {
    $build = $path_builds . DIRECTORY_SEPARATOR . $build;
  }

  // If we have settings for the base build location (not in builds directory),
  // but we build in another place, check if we can manage some symlink to it.
  if (!$dir_build || realpath($build) == realpath($path_build)) {
    return drush_log(dt('Build at !path is already activated.', array('!path' => $build)), 'success');
  }

  // If directory build is not a link
  if (is_dir($path_build) && is_link($path_build)) {
    drush_shell_exec('rm %s', $root . DIRECTORY_SEPARATOR . $dir_build);
  } elseif (is_dir($path_build)) {
    drush_log(dt('The directory ./build is NOT a symlink.
                  Please remove the current ./build directory
                  or move the ./build directory to ./builds/my-old-build'), 'error');
    return drush_set_error(dt('Could not build in a existing directory.'));
  }

  require_once dirname(__FILE__) . '/includes/kraftwagen.fileutils.inc';
  kraftwagen_symlink($build, $path_build);

  drush_log(dt('Symlinked \'!target\' to \'!link\'.', array('!link' => $root . DIRECTORY_SEPARATOR . $dir_build, '!target' => $build)), 'success');
}
