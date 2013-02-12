<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kw-generate-makefile`.
 */

/**
 * Implements drush_COMMAND_init() for `drush kw-generate-makefile`.
 */
function drush_kw_generate_makefile_init() {
  kraftwagen_context_init_kraftwagen_root(FALSE);
}

function drush_kraftwagen_kw_generate_makefile($location, $src_dir = null) {
  if (!$src_dir) {
    // Find out where the Kraftwagen root is
    $root = kraftwagen_context_get_option('root-path');

    if (!($dir_src = kraftwagen_context_get_option('src-dir'))) {
      return drush_set_error(dt('No src dir name set.'));
    }

    if (!@lstat($root . DIRECTORY_SEPARATOR . $dir_src)) {
      return drush_set_error(dt('Could not find source at !src.', array('!src' => $root . DIRECTORY_SEPARATOR . $dir_src)));
    }

    $src_dir = $root . DIRECTORY_SEPARATOR . $dir_src;
  }

  if (!($dir_src_tools = kraftwagen_context_get_option('src-tools-dir'))) {
    return drush_set_error(dt('No src tools subdir name set.'));
  }

  if (!($file_make_tpl = kraftwagen_context_get_option('make-file-tpl'))) {
    return drush_set_error(dt('No make file name set.'));
  }

  if (!is_file($src_dir . DIRECTORY_SEPARATOR . $dir_src_tools . DIRECTORY_SEPARATOR . $file_make_tpl)) {
    return drush_set_error(dt('No make file found at !path.', array('!path' => $root . DIRECTORY_SEPARATOR . $dir_src . DIRECTORY_SEPARATOR . $dir_src_tools . DIRECTORY_SEPARATOR . $file_make_tpl)));
  }

  require_once dirname(__FILE__) . '/includes/kraftwagen.fileutils.inc';
  kraftwagen_file_replace($src_dir . DIRECTORY_SEPARATOR . $dir_src_tools . DIRECTORY_SEPARATOR . $file_make_tpl, $location, array(
    '**SRC_DIR**' => $src_dir,
  ));

  drush_log(dt('Make file generated.'), 'success');
}