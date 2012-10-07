<?php

function kraftwagen_compare_files($a, $b) {
  $func = kraftwagen_context_get_option('file-hashing-function');
  return ($func($a) == $func($b));
}

/**
 * Perform a find and replace in a file and write the result to some file. We 
 * use this to execute replacement patterns in skeleton and template files.
 */
function kraftwagen_file_replace($original_filepath, $destination_filepath, $replacements) {
  $contents = file_get_contents($original_filepath);
  $contents = str_replace(array_keys($replacements), array_values($replacements), $contents);
  file_put_contents($destination_filepath, $contents);
}
