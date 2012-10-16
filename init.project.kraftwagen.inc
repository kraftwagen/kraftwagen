<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kraftwagen-project-init`.
 */

/**
 * Implements drush_COMMAND for `drush kraftwagen-project-init`.
 */
function drush_kraftwagen_project_init() {
  return kraftwagen_commands_sequence_run('project-init-commands');
}