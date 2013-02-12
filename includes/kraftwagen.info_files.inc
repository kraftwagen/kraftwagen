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

/**
 * Generate the makefile data from an info array.
 * 
 * @param $info
 *   The associative array that will be parsed into a file.
 *
 * @return
 *   String representation of the array, that can we written to a file.
 */
function kraftwagen_generate_info_file($info) {
  // TODO: write simplification to create lines like 'projects[drupal] = 7.2' 
  // instead of 'projects[drupal][version] = 7.2'.
  
  // Fetch the data as an array of lines and implode.
  return implode("\n", _kraftwagen_generate_info_file($info));
}

/**
 * Internal function that generates an array of lines for the make file, from an
 * info array. Will be called recursively.
 */
function _kraftwagen_generate_info_file($info, $path = array()) {
  $lines = array();
  
  foreach ($info as $key => $value) {
    if (is_array($value)) {
      if (empty($value)) {
        // Create a line like 'projects[] = drupal' for empty property arrays.
        $lines[] = _kraftwagen_generate_info_file_line(array_merge($path, array(NULL)), $key);
      }
      else {
        // Add newlines to keep the file a little readable.
        if (count($path) < 3 && !empty($lines) && $lines[count($lines)-1]) {
          $lines[] = '';
        }
        // Generate lines for the subarray and merge it into our result.
        $lines = array_merge($lines, _kraftwagen_generate_info_file($value, array_merge($path, array($key))));
      }
    } 
    else {
      // Create line like 'projects[drupal][version] = 7.2 for simple scalar 
      // values.
      $lines[] = _kraftwagen_generate_info_file_line(array_merge($path, array($key)), $value);
    }
  }
  
  return $lines;
}

/**
 * Internal function. Generates a makefile line for a path and a value.
 */
function _kraftwagen_generate_info_file_line($path, $value) {
  // Create key from path.
  $key = _kraftwagen_generate_info_file_key($path);
  
  return "$key = $value";
}

/**
 * Internal function. Generates a key for a path.
 */
function _kraftwagen_generate_info_file_key($path) {
  $key = '';
  foreach ($path as $part) {
    if (empty($key)) {
      // If key is still empty, we do not need to includes brackets.
      $key = $part;
    } 
    else {
      // A NULL part is interpreted like [] assignment.
      if (is_null($part)) {
        $key .= '[]';
      }
      else {
        // Add part to the key.
        $key .= '[' . $part . ']';
      }
    }
  }
  
  return $key;
}
