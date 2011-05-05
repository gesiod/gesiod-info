<?php
// $Id

/**
 * @file
 * Views handler: "Maximum sell price" argument
 */

/**
 * Filters all products with a higher price than the defined by the argument.
 */
class uc_product_handler_argument_max_sell_price extends views_handler_argument {
  function title() {
    return '';
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    unset($form['breadcrumb']);
  }
  
  function uses_breadcrumb() {
    return FALSE;
  }
	
  function query() {
    $this->ensure_my_table();
    $this->query->add_where(0, "$this->table_alias.$this->real_field <= '%d'", $this->argument);
  }
};

