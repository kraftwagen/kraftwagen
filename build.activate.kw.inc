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
function drush_kraftwagen_kw_activate_build($build) {
  // Find out where the Kraftwagen root is.
  $root = kraftwagen_context_get_option('root-path');

  $dir_build = kraftwagen_context_get_option('build-dir');
  $path_build = $root . DIRECTORY_SEPARATOR . $dir_build;

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

  // Better local development workflow.
  $env = kraftwagen_context_get_option('environment');
  if ($env == 'local') {
    $dir_src = kraftwagen_context_get_option('src-dir');
    $dir_src = $root . DIRECTORY_SEPARATOR . $dir_src;

    // Get project name.
    // @TODO: This should be in the context.
    $info_file = glob($dir_src . DIRECTORY_SEPARATOR . "*.info");
    $project_name = str_replace('.info', '', basename($info_file[0]));

    drush_shell_exec('rm -rf %s', $path_build . DIRECTORY_SEPARATOR . 'profiles' . DIRECTORY_SEPARATOR . $project_name);

    kraftwagen_symlink($dir_src, $build . DIRECTORY_SEPARATOR . 'profiles' . DIRECTORY_SEPARATOR . $project_name);
  }

  drush_log(dt('Symlinked \'!target\' to \'!link\'.', array('!link' => $root . DIRECTORY_SEPARATOR . $dir_build, '!target' => $build)), 'success');
}
