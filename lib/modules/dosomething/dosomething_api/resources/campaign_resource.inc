<?php

function _campaign_resource_definition() {
  $campaign_resource = [];
  $campaign_resource['campaigns'] = [
    'operations' => [

      'index' => [
        'help' => 'List all active campaigns.',
        'file' => [
          'type' => 'inc',
          'module' => 'dosomething_api',
          'name' => 'resources/campaign_resource'
        ],
        'callback' => '_campaign_resource_index',
        'args' => [
          [
            'name' => 'ids',
            'description' => 'Retrieve campaigns based on specific node ids.',
            'optional' => TRUE,
            'type' => 'string',
            'source' => ['param' => 'ids'],
            'default value' => NULL,
          ],
          [
            'name' => 'staff_pick',
            'description' => 'Retrieve campaigns that are staff picks.',
            'optional' => TRUE,
            'type' => 'boolean',
            'source' => ['param' => 'staff_pick'],
            'default value' => FALSE,
          ],
          [
            'name' => 'term_ids',
            'description' => 'Taxonomy term ids to filter by.',
            'optional' => TRUE,
            'type' => 'string',
            'source' => ['param' => 'term_ids'],
            'default value' => NULL,
          ],
          [
            'name' => 'count',
            'description' => 'The number of campaigns to retrieve.',
            'optional' => TRUE,
            'type' => 'int',
            'source' => ['param' => 'count'],
            'default value' => 25,
          ],
          [
            'name' => 'random',
            'description' => 'Boolean to indicate whether to retrieve campaigns in random order.',
            'optional' => TRUE,
            'type' => 'boolean',
            'source' => ['param' => 'random'],
            'default value' => FALSE,
          ],
          [
            'name' => 'page',
            'description' => 'The zero-based index of the page to get, defaults to 0.',
            'optional' => TRUE,
            'type' => 'int',
            'source' => array('param' => 'page'),
            'default value' => 1,
          ],
          [
            'name' => 'cache',
            'description' => 'Boolean to indicate whether to cache data.',
            'optional' => TRUE,
            'type' => 'boolean',
            'source' => ['param' => 'cache'],
            'default value' => TRUE,
          ],
        ],
        'access callback' => '_campaign_resource_access_alt',
        'access arguments' => ['index'],
      ],

      'retrieve' => [
        'help' => 'Retrieve a specified campaign.',
        'file' => [
          'type' => 'inc',
          'module' => 'dosomething_api',
          'name' => 'resources/campaign_resource',
        ],
        'callback' => '_campaign_resource_retrieve',
        'args' => [
          [
            'name' => 'nid',
            'description' => 'The nid of the campaign to retrieve.',
            'optional' => FALSE,
            'type' => 'int',
            'source' => ['path' => 0],
          ],
        ],
        'access callback' => '_campaign_resource_access',
        'access arguments' => ['view'],
      ],

    ],


    // @TODO: The following actions will be deprecated the future in favor of REST-ful endpoints.
    'targeted_actions' => array(
      'signup' => array(
        'help' => 'Signup a user for a campaign. POST to campaigns/123/signup',
        'file' => array('type' => 'inc', 'module' => 'dosomething_api', 'name' => 'resources/campaign_resource'),
        'callback' => '_campaign_resource_signup',
        'access callback' => '_campaign_resource_access_alt',
        'access arguments' => array('signup'),
        'access arguments append' => TRUE,
        'args' => array(
          array(
            'name' => 'nid',
            'optional' => FALSE,
            'source' => array('path' => 0),
            'type' => 'int',
            'description' => 'The nid of the Campaign node to signup for.',
          ),
          array(
            'name' => 'values',
            'optional' => FALSE,
            'source' => 'data',
            'description' => 'The Signup data',
            'type' => 'array',
          ),
        ),
      ),
      'reportback' => array(
        'help' => 'Post a user reportback for a campaign: campaigns/123/reportback',
        'file' => array('type' => 'inc', 'module' => 'dosomething_api', 'name' => 'resources/campaign_resource'),
        'callback' => '_campaign_resource_reportback',
        'access callback' => '_campaign_resource_access_alt',
        'access arguments' => array('reportback'),
        'access arguments append' => TRUE,
        'args' => array(
          array(
            'name' => 'nid',
            'optional' => FALSE,
            'source' => array('path' => 0),
            'type' => 'int',
            'description' => 'The nid of the Campaign node to post a reportback.',
          ),
          array(
            'name' => 'values',
            'optional' => FALSE,
            'source' => 'data',
            'description' => 'The Reportback data',
            'type' => 'array',
          ),
        ),
      ),
    ),
  ];
  return $campaign_resource;
}


/**
 * Determine whether the current user can access a node resource.
 *
 * @param string $op
 *   Possible values of view, signup, reportback.
 * @param array $args
 *   Resource arguments passed through from the original request.
 * @return bool
 * @see node_access()
 */
// @TODO: Function will be deprecated and currently only used in soon to be deprecated targeted actions.
function _campaign_resource_access_alt($op = 'view', $args = array()) {
  if (DOSOMETHING_REPORTBACK_LOG && $args) {
    watchdog('dosomething_api', '_campaign_resource_access_alt args:' . json_encode($args));
  }

  if ($op == 'index') {
    return TRUE;
  }

  $node = node_load($args[0]);
  if (!$node) {
    return services_error(t('No node found for @nid', array('@nid' => $args[0])), 403);
  }

  if ($op == 'view') {
    return node_access($op, $node);
  }

  if (!user_is_logged_in()) {
    return services_error(t('Must be logged in!'), 403);
  }

  if (dosomething_campaign_is_active($node)) {
    return TRUE;
    //@todo: If op==reportback and SMS Game, return 403 error.
  }

  return services_error(t('Campaign node @nid is not active.', array('@nid' => $node->nid)), 403);
}


/**
 * Determine whether the current user can access campaign resource.
 *
 * @param  string  $op
 * @return bool
 */
function _campaign_resource_access($op) {
  if ($op === 'view') {
    return TRUE;
  }

  if ($op === 'index') {
    return TRUE;
  }

  return FALSE;
}


/**
 * Retrieve and show response for campaign request.
 *
 * @param  string  $nid
 * @return object
 */
function _campaign_resource_retrieve($nid) {
  return (new CampaignTransformer)->show($nid);
}


/**
 * Retrieve and show index list response of campaigns requested.
 *
 * @param  string  $ids
 * @param  bool    $staff_pick
 * @param  string  $term_ids
 * @param  int     $count
 * @param  bool    $random
 * @param  int     $page
 * @param  bool    $cache
 * @return object
 */
function _campaign_resource_index($ids, $staff_pick, $term_ids, $count, $random, $page, $cache) {
  $parameters = [
    'ids' => $ids,
    'staff_pick' => $staff_pick,
    'term_ids' => $term_ids,
    'count' => $count,
    'random' => $random,
    'page' => $page,
    'cache' => $cache,
  ];

  return (new CampaignTransformer)->index($parameters);
}


/**
 * Callback for Campaigns Signup targeted action.
 *
 * @param int $nid
 *   The Node nid to post the signup to.
 * @param array $values
 *   The signup data to post. Expected keys:
 *   - uid: The user uid (int).  Optional, uses global $user if not set.
 *   - source (string).
 *   - transactionals (bool). Optional flag to disable sending transactional messages.
 *       Defaults to sending messages.
 */
function _campaign_resource_signup($nid, $values) {
  if (isset($values['northstar_id'])) {
    $values['uid'] = dosomething_user_get_user_by_northstar_id($values['northstar_id'])->uid;
  }

  if (!isset($values['uid'])) {
    return services_error('Cannot create signup without a `northstar_id` or `uid`.');
  }

  if (!isset($values['source'])) {
    $values['source'] = NULL;
  }
  // @TODO: handle passing this to Rogue so it is not lost (and add to Phoenix documentation which is missing this)
  if (!isset($values['transactionals'])) {
    $values['transactionals'] = TRUE;
  }
  if (DOSOMETHING_SIGNUP_LOG_SIGNUPS) {
    watchdog('dosomething_api', '_campaign_resource_signup values:' . json_encode($values));
  }

  // Check to see if we should be sending requests to Rogue instead:
  if (variable_get('rogue_collection', FALSE)) {
    $rogue_signup = dosomething_rogue_send_signup_to_rogue(array_merge($values, [
      'campaign_id' => $nid,
    ]));

    // Store the reference to the Rogue signup
    if ($rogue_signup) {
      return dosomething_rogue_check_sid_and_store_ref($rogue_signup);
    }
  }

  return dosomething_signup_create($nid, $values['uid'], $values['source'], NULL, $values['transactionals']);
}


/**
 * Callback for Campaigns Reportback targeted action.
 *
 * @param int $nid
 *   The Node nid to post the reportback to.
 * @param array $values
 *   The reportback data to post. Expected keys:
 *   - uid: The user uid (int).
 *   - file: Base64 encoded file string to save (required if using 'filename').
 *   - filename: The filename of the file provided as file (required if using 'file').
 *   - file_url: The URL of the reportback file to save (used if no 'file'/'filename' exist).
 *   - caption: The caption for the Reportback File if provided.
 *   - quantity (int).
 *   - why_participated (string).
 *   - num_participants (int).
 *   - remote_addr (string).
 */
function _campaign_resource_reportback($nid, $values) {
  global $user;
  $api_user = $user;

  if (!user_access('view any reportback', $api_user)) {
    return dosomething_helpers_basic_http_response(401, 'Unauthorized.');
  }

  if (empty($values['uid'])) {
    return dosomething_helpers_basic_http_response(422, 'Missing uid query parameter specifying a user id.');
  }

  $user = user_load(dosomething_user_convert_to_legacy_id($values['uid']));

  if (!$user) {
    return dosomething_helpers_basic_http_response(404, 'The specified user was not found.');
  }

  // Set uid to member user submitting reportback.
  $values['uid'] = $uid = $user->uid;

  // @todo: Return error if signup doesn't exist.

  // @todo: Move this logic into dosomething_reportback_save().
  $rbid = dosomething_reportback_exists($nid, NULL, $uid) ?: 0;

  // If file is submitted as an upload, the 'file' and 'filename' values must be provided.
  if (!empty($values['file']) && !empty($values['filename'])) {
    $values['filepath'] = dosomething_reportback_get_file_dest($values['filename'], $nid, $uid);

    // Use Services module's File Create resource to save the file.
    module_load_include('inc', 'services', 'resources/file_resource');

    $result = _file_resource_create($values);

    $file = file_load($result['fid']);
  }
  elseif (!empty($values['file_url'])) {
    $file = dosomething_reportback_save_file_from_url($nid, $uid, $values['file_url']);

    if (!$file) {
      return dosomething_helpers_basic_http_response(500, 'Could not write file to destination.');
    }
  } else {
    $file = NULL;
  }

  // If first time user is submitting RB, then a file is required.
  if (!$file && $rbid === 0) {
    return dosomething_helpers_basic_http_response(422, 'Cannot create new reportback without a file.');
  }

  // Update or set values.
  if ($file) {
    $values['fid'] = $file->fid;
    $values['file'] = dosomething_helpers_get_data_uri_from_fid($file->fid);
  }

  $values['nid'] = $nid;
  $values['rbid'] = $rbid;

  // Logging.
  if (DOSOMETHING_REPORTBACK_LOG) {
    watchdog('dosomething_api', '_campaign_resource_reportback:' . json_encode($values));
  }

  $headers = getallheaders();

  if (variable_get('rogue_collection', FALSE) && ! isset($headers['X-Request-Id'])) {
    $rogue_reportback = dosomething_rogue_send_reportback_to_rogue($values, $user);

    // Make sure reportback made it back to Phoenix and store reference to the Rogue item if one was added.
    $rb_info = dosomething_rogue_check_rbid_and_store_ref($rogue_reportback);

    return $rb_info['rbid'];
  }
  else {
    if (isset($headers['X-Request-Id'])) {
      // Increment transaction id and log that the request has been received with new transaction id.
      $incremented_transaction_id = dosomething_api_increment_transaction_id($headers['X-Request-Id']);

      watchdog('PhoenixTransactionBridge', 'Request received. Transaction ID: !incremented_transaction_id', ['!incremented_transaction_id' => $incremented_transaction_id], WATCHDOG_INFO);
    }

    return dosomething_reportback_save($values, $user);
  }
}
