<?php

/**
 * Read a info file from disk. Stripped down version of 
 * drush_make_parse_info_file that does not parse includes.
 */
function kraftwagen_parse_info_file($makefile) {
  // get the content of the makefile
  if (!($data = file_get_contents($makefile))) {
    return drush_set_error(dt('Invalid or empty make file: %makefile', array('%makefile' => $makefile)));
  }
  
  // parse the makefile
  if (!($info = _drush_make_parse_info_file($data))) {
    return FALSE;
  }
  
  return $info;
}


/**
 * Generate the makefile data from an info array.
 */
function kraftwagen_generate_info_file($info) {
  // TODO: write simplification to create lines like 'projects[drupal] = 7.2' 
  // instead of 'projects[drupal][version] = 7.2'
  
  // fetch the data as an array of lines and implode
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
        // create a line like 'projects[] = drupal' for empty property arrays
        $lines[] = _kraftwagen_generate_info_file_line(array_merge($path, array(NULL)), $key);
      }
      else {
        // add newlines to keep the file a little readable
        if (count($path) < 3 && !empty($lines) && $lines[count($lines)-1]) {
          $lines[] = '';
        }
        // generate lines for the subarray and merge it into our result
        $lines = array_merge($lines, _kraftwagen_generate_info_file($value, array_merge($path, array($key))));
      }
    } 
    else {
      // create line like 'projects[drupal][version] = 7.2 for simple scalar 
      // values
      $lines[] = _kraftwagen_generate_info_file_line(array_merge($path, array($key)), $value);
    }
  }
  
  return $lines;
}

/**
 * Internal function. Generates a makefile line for a path and a value.
 */
function _kraftwagen_generate_info_file_line($path, $value) {
  // create key from path
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
      // if key is still empty, we do not need to includes brackets
      $key = $part;
    } 
    else {
      // a NULL part is interpreted like [] assignment
      if (is_null($part)) {
        $key .= '[]';
      }
      else {
        // add part to the key
        $key .= '[' . $part . ']';
      }
    }
  }
  
  return $key;
}