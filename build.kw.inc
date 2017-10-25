<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kw-build`.
 */

/**
 * Implements drush_COMMAND_init() for `drush kw-build`.
 */
function drush_kw_build_init() {
  kraftwagen_context_init_kraftwagen_root();
}

/**
 * Implements drush_COMMAND() for `drush kw-build`.
 */
function drush_kraftwagen_kw_build() {
  // Find out where the Kraftwagen root is.
  $root = kraftwagen_context_get_option('root-path');

  // Generate new make file in tmp.
  require_once dirname(__FILE__) . '/includes/kraftwagen.fileutils.inc';
  $make_file_location = drush_tempnam('kw');

  // Check for the working-copy option.
  $working_copy = drush_get_option(array('working-copy')) ? '1' : '0';

  // Gind out where to create the new build.
  if (!($target_dir = kraftwagen_build_determine_target($root))) {
    return drush_set_error(dt('Could not determine a build target.'));
  }

  kraftwagen_commands_sequence_run('build-commands', array(
    '*make_file_location*' => $make_file_location,
    '*target_dir*' => $target_dir,
    '*working_copy*' => $working_copy,
  ));
}

/**
 * Find where to store a build.
 *
 * @param string $root
 *   Kraftwagen root path
 *
 * @return string
 *   The path to the directory to store the build in. This directory does not
 *   always already exist. If no target could be determined, FALSE is returned.
 */
function kraftwagen_build_determine_target($root) {
  list($dir_build, $dir_builds, $date_pattern)
    = kraftwagen_context_get_options('build-dir', 'builds-dir', 'date-pattern');

  // Find out where to store the build. Possible locations, order of preference:
  // 1. 'builds' directory. If we have a location where all builds are kept, we
  // use this. This will only work when the 'builds' directory already exists
  // and a date pattern is available.
  // 2. 'build' directory. If we have name for a directory to store a single
  // build, use this. This is the default behavior and will succeed in most
  // case. If you want to disable this logic, for example in a production
  // environment, you should set 'build-dir' to FALSE in kraftwagenrc.php.
  if ($dir_builds && is_dir($root . DIRECTORY_SEPARATOR . $dir_builds) && $date_pattern) {
    $date = date($date_pattern, time());

    // Generate a directory name that does not exist yet. If the date pattern
    // is well constructed, we rarely need to add a suffix.
    $suffix = 0;
    do {
      $target_dir = $root . DIRECTORY_SEPARATOR . $dir_builds . DIRECTORY_SEPARATOR . $date . ($suffix ? "-{$suffix}" : '');
      $suffix += 1;
    } while (is_dir($target_dir));

    return $target_dir;
  }

  // If directory build is not a link
  if (!is_link($root . DIRECTORY_SEPARATOR . $dir_build)) {
    drush_log(dt('The directory ./build is NOT a symlink.'. PHP_EOL
                .'Please remove the current ./build directory'. PHP_EOL
                .'or move the ./build directory to ./builds/my-old-build'. PHP_EOL), 'error');
  }

  return FALSE;
}

