<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kw-import-translations`.
 */

/**
 * Implements drush_COMMAND for `drush kw-import-translations`.
 */
function drush_kraftwagen_kw_import_translations() {
  include_once DRUPAL_ROOT . '/includes/locale.inc';

  $files = kraftwagen_import_translations_files();
  if (!$files) {
    return drush_log(dt('No translation files found.'), 'warning');
  }

  foreach ($files as $file) {
    $path = realpath($file->uri);
    if (!kraftwagen_import_translations_import($path)) {
      return drush_error(dt('Could not import @path', array('@path' => $path)));
    }
    drush_log(dt('Imported @path', array('@path' => $path)), 'status');
  }

  drush_log(dt('Translation files imported.'), 'success');
}

/**
 * Find out which .po files to import.
 */
function kraftwagen_import_translations_files() {
  $files = array();
  $languages = language_list('enabled');
  unset($languages[1]['en']);
  if (count($languages[1])) {
    $language_list = join('|', array_keys($languages[1]));

    // Drupal core.
    $files = array_merge($files, file_scan_directory(DRUPAL_ROOT . '/translations', '/(^|\.)(' . $language_list . ')\.po$/', array('recurse' => FALSE)));

    // Collect all files to import for all components. Profile will be imported last.
    $result = db_query("SELECT name, filename FROM system WHERE status = 1 ORDER BY (type = 'profile'), weight, name");
    foreach ($result as $component) {
      // Collect all files for this component in all enabled languages, named
      // as $langcode.po or with names ending with $langcode.po. This allows
      // for filenames like node-module.de.po to let translators use small
      // files and be able to import in smaller chunks.
      $files = array_merge($files, file_scan_directory(dirname($component->filename) . '/translations', '/(^|\.)(' . $language_list . ')\.po$/', array('recurse' => FALSE)));
    }

    return $files;
  }
  return FALSE;
}

/**
 * Import a .po file.
 */
function kraftwagen_import_translations_import($filepath) {
  if (preg_match('!(/|\.)([^\./]+)\.po$!', $filepath, $langcode)) {
    $file = (object) array('filename' => drupal_basename($filepath), 'uri' => realpath($filepath));
    _locale_import_read_po('db-store', $file, LOCALE_IMPORT_OVERWRITE, $langcode[2]);
    return TRUE;
  }

  return FALSE;
}
