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
  // find out where the Kraftwagen root is
  $root = kraftwagen_context_get_option('root-path');

  // generate new make file in tmp
  require_once dirname(__FILE__) . '/includes/kraftwagen.fileutils.inc';
  $make_file_location = drush_tempnam('kw');


  // find out where to create the new build
  if (!($target_dir = kraftwagen_build_determine_target($root))) {
    return drush_set_error(dt('Could not determine a build target.'));
  }

  // get a maybe existant build out of the way
  if (!($result = kraftwagen_build_manage_existing_target($root, $target_dir))) {
    return $result;
  }

  kraftwagen_commands_sequence_run('build-commands', array(
    '*make_file_location*' => $make_file_location,
    '*target_dir*' => $target_dir,
  ));
}

/**
 * Find where to store a build.
 *
 * @param $root
 *   Kraftwagen root path
 *
 * @return 
 *   The path to the directory to store the build in. This directory does not 
 *   always already exist. If no target could be determined, FALSE is returned.
 */
function kraftwagen_build_determine_target($root) {
  list($dir_build, $dir_builds, $date_pattern) = 
    kraftwagen_context_get_options('build-dir', 'builds-dir', 'date-pattern');

  // Find out where to store the build. Possible locations, order of preference:
  // 1. 'builds' directory. If we have a location where all builds are kept, we
  //    use this. This will only work when the 'builds' directory already 
  //    exists and a date pattern is available.
  // 2. 'build' directory. If we have name for a directory to store a single
  //    build, use this. This is the default behavior and will succeed in most
  //    case. If you want to disable this logic, for example in a production
  //    environment, you should set 'build-dir' to FALSE in kraftwagenrc.php.
  if ($dir_builds && is_dir($root . DIRECTORY_SEPARATOR . $dir_builds) && $date_pattern) {
    $date = date($date_pattern, time());

    // Generate a directory name that does not exist yet. If the date pattern
    // is well constructed, we rarely need to add a suffix
    $suffix = 0;
    do {
      $target_dir = $root . DIRECTORY_SEPARATOR . $dir_builds . DIRECTORY_SEPARATOR . $date . ($suffix ? "-{$suffix}" : '');
      $suffix += 1;
    } while (is_dir($target_dir));

    return $target_dir;
  }

  if ($dir_build) {
    // we don't care whether the build directory does already exist
    return $root . DIRECTORY_SEPARATOR . $dir_build;
  }

  return FALSE;
}

/**
 * Remove and optionally archive an existing build target.
 *
 * It will check if the target exists, and if it does, it tries to archive it 
 * and removes it.
 * 
 * @param $root
 *   Kraftwagen root path
 * @param $target_dir
 *   The, maybe existing target dir
 *
 * @return 
 *   TRUE on success, FALSE on failure
 */
function kraftwagen_build_manage_existing_target($root, $target_dir) {
  // jump out if we do not have an existing target
  if (!is_dir($target_dir)) {
    return true;
  }

  $dir_build = kraftwagen_context_get_option('build-dir');

  // if not, jump out
  if (!$result) {
    return drush_user_abort(dt('Build aborted, because we can\'t overwrite existing build target'));
  }

  // first try to archive the existing build
  require_once dirname(__FILE__) . '/includes/kraftwagen.archive.inc';
  $archive_dest = kraftwagen_archive_directory($dir_build ? $dir_build : 'build', $target_dir);
  if ($archive_dest) {
    drush_log(dt('Build archived to !dest.', array('!dest' => $archive_dest)), 'success');
  } else {
    return drush_set_error(dt('Could not archive build.'));
  }

  // remove the existing build
  drush_shell_exec('find %s/sites -type d -depth 1 | xargs chmod +w', $target_dir);
  drush_shell_exec('rm -rf %s', $target_dir);
  return true;
}
