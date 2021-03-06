<?php

class CampaignController {

  /**
   * Update the specified Campaign in storage.
   *
   * @param  int  $id
   * @return void
   */
  function update($id) {
    cache_clear_all('ds_campaign_' . $id, 'cache_dosomething_campaign', TRUE);
    cache_clear_all('campaigns_show_api_request|id=' . $id, 'cache_dosomething_api');
    cache_clear_all('campaigns_index_api_request', 'cache_dosomething_api', TRUE);
  }

}
