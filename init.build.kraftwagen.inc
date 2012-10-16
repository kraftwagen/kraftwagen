<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kraftwagen-build-init`.
 */

/**
 * Implements drush_COMMAND for `drush kraftwagen-build-init`.
 */
function drush_kraftwagen_build_init() {
  return kraftwagen_commands_sequence_run('build-init-commands');
}