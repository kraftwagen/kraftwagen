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

/**
 * Helper to get a relative path from 2 absolute paths.
 *
 * This function is taken from http://stackoverflow.com/a/2638272
 *
 * @param $from
 *   The directory to start from.
 * @param $to
 *   The directory to go to.
 *
 * @return string
 *   The relative path.
 */
function kraftwagen_get_relative_path($from, $to) {
  // Some compatibility fixes for Windows paths.
  $from = is_dir($from) ? rtrim($from, '\/') . '/' : $from;
  $to   = is_dir($to)   ? rtrim($to, '\/') . '/'   : $to;
  $from = str_replace('\\', '/', $from);
  $to   = str_replace('\\', '/', $to);

  $from     = explode('/', $from);
  $to       = explode('/', $to);

  $rel_path  = $to;
  foreach ($from as $depth => $dir) {
    // Find first non-matching dir.
    if ($dir === $to[$depth]) {
      // Ignore this directory.
      array_shift($rel_path);
      continue;
    }

    // Get number of remaining dirs to $from.
    $remaining = count($from) - $depth;
    if ($remaining > 1) {
      // Add traversals up to first matching dir.
      $pad_length = (count($rel_path) + $remaining - 1) * -1;
      $rel_path = array_pad($rel_path, $pad_length, '..');
      break;
    }
    
    $rel_path[0] = './' . $rel_path[0];
  }

  return implode('/', $rel_path);
}

function kraftwagen_symlink($from, $to) {
  // Some compatibility fixes for Windows paths.
  $from = is_dir($from) ? rtrim($from, '\/') . '/' : $from;
  $to   = is_dir($to)   ? rtrim($to, '\/') . '/'   : $to;
  $from = str_replace('\\', '/', $from);
  $to   = str_replace('\\', '/', $to);

  // Clear statcache for build directory, PHP tells us our symlink exists because we execute realpath().
  clearstatcache(TRUE, $to);

  // Get old working directory.
  $old_cwd = getcwd();

  // Find the directory where the symlink should be placed in, and change to it.
  $to_parent = dirname($to);
  chdir($to_parent);

  $use_relative_symlinks = kraftwagen_context_get_option('use-relative-symlinks');
  $target = $to;
  if ($use_relative_symlinks) {
    $target = kraftwagen_get_relative_path($to_parent, $from);
  }

  symlink($target, $to);

  // Change working directory back.
  chdir($old_cwd);
}
