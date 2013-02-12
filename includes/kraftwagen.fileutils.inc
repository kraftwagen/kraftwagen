<?php

/**
 * @file
 * This file provides a few helper functions for file management.
 */

/**
 * Check if two files are equal.
 *
 * @param $a
 *   Filename of the first file
 * @param $b
 *   Filename of the second file
 *
 * @return
 *   TRUE if the two files have the same content.
 */
function kraftwagen_compare_files($a, $b) {
  $func = kraftwagen_context_get_option('file-hashing-function');
  return ($func($a) == $func($b));
}

/**
 * Perform a find and replace in a file and write the result to some other file.
 * We use this to execute replacement patterns in skeleton and template files.
 *
 * @param $original_filepath
 *   The file where the template is stored.
 * @param $destination_filepath
 *   The file where the result should be stored
 * @param $replacements
 *   Associative array that provides a mapping between strings that are in the
 *   template file and how they should end up in the result.
 */
function kraftwagen_file_replace($original_filepath, $destination_filepath, $replacements) {
  $contents = file_get_contents($original_filepath);
  $contents = str_replace(array_keys($replacements), array_values($replacements), $contents);
  file_put_contents($destination_filepath, $contents);
}
