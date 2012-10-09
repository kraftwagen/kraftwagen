<?php

/**
 * Implements make_download_DOWNLOAD_TYPE() for 'kraftwagen_directory'. 
 * This allows building directly from the filesystem, instead of from a 
 * repository.
 */
function make_download_kraftwagen_directory($name, $download, $download_location) {
  $result = drush_copy_dir($download['url'],$download_location, FILE_EXISTS_OVERWRITE);
  return $result ? $download_location : FALSE;
}
