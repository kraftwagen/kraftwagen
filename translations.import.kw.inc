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

  $batch = kraftwagen_import_translations_batch();

  if (!$batch) {
    return drush_log(dt('No translation files found.'), 'warning');
  }

  batch_set($batch);
  $batch = &batch_get();
  $batch['progressive'] = FALSE;
  drush_backend_batch_process();

  drush_log(dt('Translation files imported.'), 'success');
}

/**
 * Constructs the batch using the locale functionality in Drupal
 */
function kraftwagen_import_translations_batch() {
  $files = array();
  $languages = language_list('enabled');
  unset($languages[1]['en']);
  if (count($languages[1])) {
    $language_list = join('|', array_keys($languages[1]));

    // Drupal core
    $files = array_merge($files, file_scan_directory(DRUPAL_ROOT . '/translations', '/(^|\.)(' . $language_list . ')\.po$/', array('recurse' => FALSE)));

    // Collect all files to import for all components. Profile will be imported last
    $result = db_query("SELECT name, filename FROM system WHERE status = 1 ORDER BY (type = 'profile'), weight, name");
    foreach ($result as $component) {
      // Collect all files for this component in all enabled languages, named
      // as $langcode.po or with names ending with $langcode.po. This allows
      // for filenames like node-module.de.po to let translators use small
      // files and be able to import in smaller chunks.
      $files = array_merge($files, file_scan_directory(dirname($component->filename) . '/translations', '/(^|\.)(' . $language_list . ')\.po$/', array('recurse' => FALSE)));
    }

    return _locale_batch_build($files);
  }
  return FALSE;
}