<?php

/**
 * @file
 *   Core drush output formats.
 */

/**
 * Implementation of hook_drush_engine_type_info().
 */
function outputformat_drush_engine_type_info() {
  $info = array();
  $info['outputformat'] = array(
    'description' => 'Output formatting options selection and use.',
    'topic' => 'docs-output-formats',
    'topic-file' => 'docs/output-formats.html',
    'combine-help' => 'TRUE',
    'option' => 'format',
    'options' => array(
      'format' => array(
        'description' => 'Select output format to use.',
      ),
      'fields' => array(
        'description' => 'Fields to output.',
        'example-value' => 'field1,field2',
        'value' => 'required',
      ),
    ),
    // Allow output formats to declare their
    // "output data type" instead of their
    // "required engine capability" for readability.
    'config-aliases' => array(
      'output-data-type' => 'require-engine-capability',
    ),
  );
  return $info;
}

/**
 * Implements hook_drush_engine_ENGINE_TYPE().
 *
 * The output format types supported are represented by
 * the 'engine-capabilities' of the output format engine.
 * The different capabilities include:
 *
 * format-single:       A simple string.
 *
 * format-list:         An associative array where the key
 *                      is usualy the row label, and the value
 *                      is a simple string.  Some list formatters
 *                      render the label, and others (like
 *                      "list" and "csv") throw it away.
 *
 * format-table:        An associative array, where the key
 *                      is the row id, and the value is the
 *                      column data.  The column data is also
 *                      an associative array where the key
 *                      is the column id and the value is the
 *                      cell data.  The cell data should usually
 *                      be a simple string; however, some
 *                      formatters can recursively format their
 *                      cell contents before rendering (e.g. if
 *                      a cell contains a list of items in an array).
 *
 * These definitions align with the declared 'output-data-type'
 * declared in command records.  @see drush_parse_command().
 *
 * Any output format that does not declare any engine capabilities
 * is expected to be able to render any php data structure that is
 * passed to it.
 */
function outputformat_drush_engine_outputformat() {
  $common_topic_example = array(
    "a" => array("b" => 2, "c" => 3),
    "d" => array("e" => 5, "f" => 6)
  );

  $engines = array();
  $engines['table'] = array(
    'description' => 'A formatted, word-wrapped table.',
    'engine-capabilities' => array('format-table'),
  );
  $engines['key-value'] = array(
    'description' => 'A formatted list of key-value pairs.',
    'engine-capabilities' => array('format-single', 'format-list', 'format-table'),
  );
  $engines['key-value-list'] = array(
    'engine-class' => 'list',
    'list-item-type' => 'key-value',
    'description' => 'A list of formatted lists of key-value pairs.',
    'engine-capabilities' => array('format-table'),
  );
  $engines['json'] = array(
    'machine-parsable' => TRUE,
    'description' => 'Javascript Object Notation.',
    'verbose-only' => TRUE,
    'topic-example' => $common_topic_example,
  );
  $engines['string'] = array(
    'machine-parsable' => TRUE,
    'description' => 'A simple string.',
    'engine-capabilities' => array('format-single'),
  );
  $engines['print-r'] = array(
    'machine-parsable' => TRUE,
    'description' => 'Output via php print_r function.',
    'verbose-only' => TRUE,
    'topic-example' => $common_topic_example,
  );
  $engines['ini'] = array(
    'machine-parsable' => TRUE,
    'description' => 'A list of key-value pairs, parsable by php parse_ini_file().',
    'engine-capabilities' => array('format-list', 'format-table'),
  );
  $engines['ini-sections'] = array(
    'machine-parsable' => TRUE,
    'engine-class' => 'list',
    'list-item-type' => 'ini',
    'description' => 'A sectioned list of key-value pairs, parsable by php parse_ini_file().',
    'engine-capabilities' => array('format-table'),
  );
  $engines['export'] = array(
    'machine-parsable' => TRUE,
    'description' => 'An array in executable php format.',
    'verbose-only' => TRUE,
    'topic-example' => $common_topic_example,
  );
  $engines['config'] = array(
    'machine-parsable' => TRUE,
    'engine-class' => 'list',
    'list-item-type' => 'export',
    'description' => "A configuration file in executable php format. The variable name is \"config\", and the variable keys are taken from the output data array's keys.",
    'metadata' => array(
      'variable-name' => 'config',
    ),
    'engine-capabilities' => array('format-list','format-table'),
    'verbose-only' => TRUE,
  );
  $engines['list'] = array(
    'machine-parsable' => TRUE,
    'list-item-type' => 'string',
    'description' => 'A simple list of values.',
    'engine-capabilities' => array('format-list'),
    'topic-example' => array('a', 'b', 'c'),
  );
  $engines['csv'] = array(
    'machine-parsable' => TRUE,
    'engine-class' => 'list',
    'list-separator' => ',',
    'list-item-type' => 'string',
    'description' => 'A comma-separated list of values.',
    'engine-capabilities' => array('format-list'),
    'topic-example' => array('a', 'b', 'c'),
  );
  $engines['variables'] = array(
    'machine-parsable' => TRUE,
    'description' => 'A list of php variable assignments.',
    'engine-capabilities' => array('format-table'),
    'verbose-only' => TRUE,
    'topic-example' => $common_topic_example,
  );
  return $engines;
}

/**
 * Implements hook_drush_command_alter
 */
function outputformat_drush_command_alter(&$command) {
  // In --pipe mode, change the default format
  // to the default pipe format, or to json, if
  // no default pipe format is given.
  if (drush_get_context('DRUSH_PIPE') && (isset($command['engines']['outputformat']))) {
    $default_format = isset($command['engines']['outputformat']['pipe-format']) ? $command['engines']['outputformat']['pipe-format'] : 'json';
    $command['engines']['outputformat']['default'] = $default_format;
  }
}

/**
 * Implements hook_drush_help_alter().
 */
function outputformat_drush_help_alter(&$command) {
  if (isset($command['engines']['outputformat'])) {
    $outputformat = $command['engines']['outputformat'];
    // If the command defines specific field labels,
    // then modify the help for --fields to include
    // specific information about the available fields.
    if (isset($outputformat['field-labels'])) {
      $all_fields = array_keys($outputformat['field-labels']);
      $field_defaults = isset($outputformat['fields-default']) ? $outputformat['fields-default'] : $all_fields;
      $command['options']['fields']['example-value'] = implode(', ', $field_defaults);
      $command['options']['fields']['description'] .= ' '. dt('All available fields are: !fields.', array('!fields' => implode(', ', $all_fields)));
      if (isset($outputformat['fields-default'])) {
        $command['options']['full']['description'] = dt("Show the full output, with all fields included.");
      }
    }
    else {
      // If the command does not define specific field labels,
      // then hide the help for --fields unless the command
      // works with output format engines that format tables.
      if (isset($outputformat['require-engine-capability'])) {
        if (!in_array('format-table', $outputformat['require-engine-capability'])) {
          unset($command['options']['fields']);
        }
      }
    }
    // If the command defines a default pipe format, then
    // add '--pipe   Equivalent to --format=<default>'.
    if (isset($outputformat['pipe-format'])) {
      if (isset($command['options']['pipe'])) {
        $command['options']['pipe'] .= ' ';
      }
      else {
        $command['options']['pipe'] = '';
      }
      $command['options']['pipe'] .= dt("Equivalent to --format=!default.", array('!default' => $outputformat['pipe-format']));
    }
  }
}

/**
 * Implements hook_drush_engine_topic_additional_text().
 */
function outputformat_drush_engine_topic_additional_text($engine, $instance, $config) {
  $result = array();

  // If the output format engine has a 'topic-example' in
  // its configuration, then format the provided array using
  // the output formatter, and insert the result of the
  // transform into the topic text.
  if ($engine == 'outputformat') {
    if (array_key_exists('topic-example', $config)) {
      $code = $config['topic-example'];
      $formatted = drush_format($code, array(), $instance);
      $result[] = dt("Code:\n\nreturn !code;\n\nOutput with --format=!instance:\n\n!formatted", array('!code' => var_export($code, TRUE), '!instance' => $instance, '!formatted' => $formatted));
    }
  }

  return $result;
}

/**
 * Interface for output format engines.
 */
class drush_outputformat {
  function __construct($config) {
    $config += array(
      'column-widths' => array(),
      'engine-info' => array(),
    );
    $config['engine-info'] += array(
      'machine-parsable' => FALSE,
      'metadata' => array(),
    );
    $config += $config['engine-info']['metadata'];
    $this->engine_config = $config;
  }
  function format_error($message) {
    return drush_set_error('DRUSH_FORMAT_ERROR', dt("The output data could not be processed by the selected format '!type'.  !message", array('!type' => $this->selected_engine, '!message' => $message)));
  }
  function process($input, $metadata = array()) {
    $metadata += $this->engine_config;
    if (isset($metadata['private-fields']) && is_array($input)) {
      if (!drush_get_option('show-passwords', FALSE)) {
        if (!is_array($metadata['private-fields'])) {
          $metadata['private-fields'] = array($metadata['private-fields']);
        }
        foreach ($metadata['private-fields'] as $private) {
          drush_unset_recursive($input, $private);
        }
      }
    }
    if (isset($metadata['fields']) && is_array($input)) {
      $first_value = reset($input);
      if (is_array($first_value)) {
        $input_data_keys = array_keys($first_value);
        $all_field_labels = (isset($metadata['field-labels']) ? $metadata['field-labels'] : array()) + array_combine($input_data_keys, $input_data_keys);
        $field_labels = array();
        foreach ($metadata['fields'] as $field) {
          if (array_key_exists($field, $all_field_labels)) {
            $field_labels[$field] = $all_field_labels[$field];
          }
        }
        // Ignore the fields option unless it selects at least one field
        if (!empty($field_labels)) {
          $metadata['field-labels'] = $field_labels;
        }
      }
    }
    if (isset($metadata[$this->selected_engine . '-metadata'])) {
      $metadata += $metadata[$this->selected_engine . '-metadata'];
    }
    $machine_parsable = $this->engine_config['engine-info']['machine-parsable'];
    $formatter_type = $machine_parsable ? 'parsable' : 'formatted';
    if ((!$machine_parsable) && is_bool($input)) {
      $input = $input ? 'TRUE' : 'FALSE';
    }
    if (isset($metadata[$formatter_type . '-filter'])) {
      $filters = $metadata[$formatter_type . '-filter'];
      if (!is_array($filters)) {
        $filters = array($filters);
      }
      foreach ($filters as $filter) {
        if (function_exists($filter)) {
          $input = $filter($input, $metadata);
        }
      }
    }
    return $this->format($input, $metadata);
  }
  function format($input, $metadata) {
    return $input;
  }
}