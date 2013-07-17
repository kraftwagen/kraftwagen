<?php

/**
 * @file
 * This file provides a few helper functions for management of files that follow
 * the Drupal .info file structure. We use it for parsing .make files.
 */

/**
 * Read a info file from disk. Stripped down version of
 * make_parse_info_file that does not parse includes.
 *
 * @param $makefile
 *   The location of the file that should be parsed.
 *
 * @return
 *   An assiociative array that represents the file. FALSE on failure.
 */
function kraftwagen_parse_info_file($makefile) {
  // Fet the content of the makefile.
  if (!($data = file_get_contents($makefile))) {
    return drush_set_error(dt('Invalid or empty make file: %makefile', array('%makefile' => $makefile)));
  }

  // Parse the makefile.
  if (!($info = _drush_drupal_parse_info_file($data))) {
    return drush_set_error(dt('Could not parse make file: %makefile', array('%makefile' => $makefile)));
  }

  return $info;
}

function kraftwagen_read_info_file($makefile) {
  $info = make_parse_info_file($makefile);
  if (!$info) {
    return FALSE;
  }

  $include_only = array(
    'projects' => array_filter(drush_get_option_list('projects')),
  );
  $info = make_prune_info_file($info, $include_only);

  if ($info === FALSE || ($info = make_validate_info_file($info)) === FALSE) {
    return FALSE;
  }

  return $info;
}
