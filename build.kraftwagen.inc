<?php

/**
 * @file
 * This file contains the functions that are required to execute 
 * `drush kraftwagen-build`.
 */

/**
 * Implements drush_COMMAND_init() for `drush kraftwagen-build`.
 */
function drush_kraftwagen_build_init() {
  kraftwagen_context_init_kraftwagen_root();
}

/**
 * Implements drush_COMMAND() for `drush kraftwagen-build`.
 */
function drush_kraftwagen_build() {
  // find out where the Kraftwagen root is
  $dir = kraftwagen_context_get_option('root-path');

  // determine source directory
  if (!($dir_src = kraftwagen_context_get_option('src-dir'))) {
    return drush_set_error(dt('No src dir name set.')); 
  }

  // find out where to create the new build
  if (!($target_dir = kraftwagen_build_determine_target($dir))) {
    return drush_set_error(dt('Could not determine a build target.'));
  }

  // find out where the make file should be
  if (!($make_file = kraftwagen_build_determine_makefile_destination($dir))) {
    return drush_set_error(dt('Could not determine a make file destination.'));
  }

  // try to create a new make file
  if ($generated_make_file = kraftwagen_build_makefile_from_template($dir . DIRECTORY_SEPARATOR . $dir_src)) {
    if (!($result = kraftwagen_build_makefile_manage_generated($dir, $generated_make_file, $make_file))) {
      return $result;
    }
  } else {
    if (is_file($make_file)) {
      drush_log(dt('Could not generate new make file. The current one will be used, but your source should contain a template.'), 'notice');
    } else {
      return drush_set_error(dt('Could not find or create a make file.'));
    }
  }

  // run the actual build
  if (!($build_dir = kraftwagen_build_create_build($make_file, $target_dir))) {
    return $build_dir;
  }

  // get a maybe existant build out of the way
  if (!($result = kraftwagen_build_manage_existing_target($dir, $target_dir))) {
    return $result;
  }

  // move the build into place
  if (!$result = make_move_build($target_dir)) {
    return false;
  }

  // if we can create a symlink to a target, try it
  kraftwagen_build_manage_symlink($dir, $target_dir);
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
 * Find out where to store the make file.
 *
 * @param $root
 *   Kraftwagen root path
 *
 * @return
 *   The path to the make file location. The file does not always already exist.
 *   If no destination could be determined, FALSE is returned.
 */
function kraftwagen_build_determine_makefile_destination($root) {
  list($dir_tools, $file_make) = 
    kraftwagen_context_get_options('tools-dir', 'make-file');

  // if we already have a make file in the tools dir, use that path
  if ($dir_tools && is_dir($root . DIRECTORY_SEPARATOR . $dir_tools) && is_file($root . DIRECTORY_SEPARATOR . $dir_tools . DIRECTORY_SEPARATOR . $file_make)) {
    return $root . DIRECTORY_SEPARATOR . $dir_tools . DIRECTORY_SEPARATOR . $file_make;
  }

  // if we already have a make file in the kraftwagen root, use that path
  if (is_file($root . DIRECTORY_SEPARATOR . $file_make)) {
    return $root . DIRECTORY_SEPARATOR . $file_make;
  }

  // if no existing make file is found, but we have a tools dir, use the path
  // of the make file in the tools dir
  if ($dir_tools && is_dir($root . DIRECTORY_SEPARATOR  . $dir_tools)) {
    return $root . DIRECTORY_SEPARATOR . $dir_tools . DIRECTORY_SEPARATOR . $file_make;
  }

  // if still no valid make file path, create it in the kraftwagen root
  return $root . DIRECTORY_SEPARATOR . $file_make;
}

/**
 * Create a new make file, from a given source directory. 
 *
 * The make file will be created in a temporary file.
 *
 * @param $src_dir
 *   The Kraftwagen source directory path.
 *
 * @return
 *   The path to where the new make file is stored. If no make file could be
 *   generated, usually because we have no template in the source directory, 
 *   FALSE will be returned.
 */
function kraftwagen_build_makefile_from_template($src_dir) {
  list($dir_src_tools, $file_make_tpl, $dir_archive) = 
    kraftwagen_context_get_options('src-tools-dir', 'make-file-tpl', 'archive-dir');

  // try to generate a make file from a make file template
  if (!$dir_src_tools || !$file_make_tpl) {
    return FALSE;
  } 
  if (!is_file($src_dir . DIRECTORY_SEPARATOR . $dir_src_tools . DIRECTORY_SEPARATOR . $file_make_tpl)) {
    return FALSE;
  }

  // generate new make file in tmp
  require_once dirname(__FILE__) . '/kraftwagen.fileutils.inc';
  $tmp_file = drush_tempnam('kw');
  kraftwagen_file_replace($src_dir . DIRECTORY_SEPARATOR . $dir_src_tools . DIRECTORY_SEPARATOR . $file_make_tpl, $tmp_file, array(
    '**SRC_DIR**' => $src_dir,
  ));
  return $tmp_file;
}

/**
 * Try to copy a generated make file to the correct location.
 *
 * Checks if the current make file exists. If it exists it asks permission to
 * overwrite it and tries to archive it. After that, it will copy the generated
 * file to the correct location.
 *
 * @param $root
 *   Kraftwagen root path
 * @param $generated_make_file
 *   Path to the newly generated make file
 * @param $current_make_file
 *   Path to where the current make file is, or where it should be stored.
 * 
 * @return 
 *   TRUE on success. FALSE on failure.
 */
function kraftwagen_build_makefile_manage_generated($root, $generated_make_file, $current_make_file) {
  if (is_file($current_make_file)) {

    require_once dirname(__FILE__) . '/kraftwagen.fileutils.inc';
    if (kraftwagen_compare_files($generated_make_file, $current_make_file)) {
      drush_log(dt('New make file generated, but it turned out to be the same as the existing file.'), 'notice');
      return true;
    }

    $dir_archive = kraftwagen_context_get_option('archive-dir');
    $can_archive = $dir_archive && is_dir($root . DIRECTORY_SEPARATOR . $dir_archive);

    // ask if we can overwrite the existing file
    $result = kraftwagen_context_confirm('make-file-overwrite', 
      $can_archive ? 
        dt('New make file generated. Do you want to overwrite the existing make file? It will be archived in the !archive directory.', array('!archive' => $dir_archive)) : 
        dt('New make file generated. Do you want to overwrite the existing make file?')
    );

    // if we should not overwrite, we also should not archive. Jump out of this 
    // function.
    if (!$result) {
      return true;
    }

    // if we may, do so
    if ($result) {
      // first try to archive the existing file
      if ($can_archive) {
        require_once dirname(__FILE__) . '/kraftwagen.archive.inc';
        if (!($archive_dest = kraftwagen_archive_file($file_make, $current_make_file))) {
          return drush_set_error(dt('Could not archive make file.'));
        }

        drush_log(dt('Make file archived to !dest.', array('!dest' => $archive_dest)), 'success');
      }
    }
  }

  // copy the new file
  if (!drush_op('copy', $generated_make_file, $current_make_file)) {
    return drush_set_error(dt('Could not save new make file.'));
  }

  drush_log(dt('New make file saved.'), 'success');
  return true;
}

/**
 * Perform the actual build.
 * 
 * @param $make_file
 *   The location of the build.make
 * @param $target_dir
 *   Where the build eventually will end up. This function will NOT put the 
 *   build there.
 *
 * @return
 *   The location where the build is created.
 */
function kraftwagen_build_create_build($make_file, $target_dir) {
  drush_include_engine('release_info', 'updatexml');

  // Get the temporary directory to create the build in. We will move it to the 
  // target when the process is finished.
  $build_dir = make_tmp();

  // check the validity of the make file
  $info = make_parse_info_file($make_file);
  if ($info === FALSE) {
    return drush_set_error(dt('Could not parse make file !file', array('!file' => $make_file)));
  }
  $info = make_validate_info_file($info);
  if ($info === FALSE) {
    return drush_set_error(dt('Make file !file is not valid', array('!file' => $make_file)));
  }

  // build the project
  $make_result = make_projects(FALSE, drush_get_option('contrib-destination', 'sites/all'), $info, $target_dir);
  if (!$make_result) {
    return drush_set_error(dt('Making of projects failed.'));
  }
  make_libraries(drush_get_option('contrib-destination', 'sites/all'), $info, $target_dir);

  return $build_dir;
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

  list($dir_build, $dir_archive) = 
    kraftwagen_context_get_options('build-dir', 'archive-dir');

  $can_archive = $dir_archive && is_dir($root . DIRECTORY_SEPARATOR . $dir_archive);

  $result = kraftwagen_context_confirm('build-overwrite', 
    $can_archive?
      dt('New build generated. Do you want to overwrite the existing build !build? It will be archived in the !archive directory.', array('!build' => $target_dir, '!archive' => $dir_archive)):
      dt('New build generated. Do you want to overwrite the existing build !build?', array('!build' => $target_dir))
  );

  // if not, jump out
  if (!$result) {
    return drush_user_abort(dt('Build aborted, because we can\'t overwrite existing build target'));
  }

  // first try to archive the existing build
  if ($can_archive) {
    require_once dirname(__FILE__) . '/kraftwagen.archive.inc';
    $archive_dest = kraftwagen_archive_directory($dir_build ? $dir_build : 'build', $target_dir);
    if ($archive_dest) {
      drush_log(dt('Build archived to !dest.', array('!dest' => $archive_dest)), 'success');
    } else {
      return drush_set_error(dt('Could not archive build.'));
    }
  }

  // remove the existing build
  drush_shell_exec('find %s/sites -type d -depth 1 | xargs chmod +w', $target_dir);
  drush_shell_exec('rm -rf %s', $target_dir);
  return true;
}

/**
 * Manage an, optionally existing symlink to the build target and create a new
 * one.
 *
 * Find out where the symlink should be created. If it exists, ask for 
 * permission to remove it. If it does not exist or with permission, create a
 * new symlink to the build target.
 *
 * @param $root
 *   Kraftwagen root path
 * @param $target
 *   The build target path
 */
function kraftwagen_build_manage_symlink($root, $target_dir) {
  $dir_build = kraftwagen_context_get_option('build-dir');

  // if we have settings for the base build location (not in builds directory),
  // but we build in another place, check if we can manage some symlink to it
  if (!$dir_build || $target_dir == ($root . DIRECTORY_SEPARATOR . $dir_build)) {
    return;
  }

  // if base build is non-existant
  // @TODO: Find out if there is a better way then !@lstat() to check if a
  // path is not taken. lstat is probably a pretty IO intensive operation, 
  // while the required information is actually very simple. 
  if (!@lstat($root . DIRECTORY_SEPARATOR . $dir_build)) {
    $result = kraftwagen_context_confirm('build-link-create', dt('Do you want a symlink !link to be created that points to your newly created build?', array('!link' => $root . DIRECTORY_SEPARATOR . $dir_build)));
  } elseif (is_link($root . DIRECTORY_SEPARATOR . $dir_build)) {
    $result = kraftwagen_context_confirm('build-link-create', dt('Do you want to overwrite the existing symlink !link to point it to your newly created build?', array('!link' => $root . DIRECTORY_SEPARATOR . $dir_build)));

    if ($result) {
      drush_shell_exec('rm %s', $root . DIRECTORY_SEPARATOR . $dir_build);
    }
  }
    
  if ($result) {
    symlink($target_dir, $root . DIRECTORY_SEPARATOR . $dir_build);
    drush_log(dt('Symlink created from !link to !target.', array('!link' => $root . DIRECTORY_SEPARATOR . $dir_build, '!target' => $target_dir)), 'success');
  }
}
