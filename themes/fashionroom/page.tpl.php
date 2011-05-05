<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="<?php print $language->language; ?>" lang="<?php print $language->language; ?>" dir="<?php print $language->dir; ?>">

<head>
  <title><?php print $head_title; ?></title>
  <?php print $head; ?>
  <?php print $styles; ?>
  <?php print $scripts; ?>

<!--[if lte IE 6]>
<style>
.block, .whiteblock, .dialog {
    position: absolute;
}
</style>
<![endif]-->
</head><body class="ff win"><a name="top"></a>

<div style="width:auto" id="body"><div id="stripe"></div>
<div align="left" style="width:100%" id="header" class="has_search">
    <table align="left" id="nav" width="99%" border="0" cellpadding="0" cellspacing="0">
        <tbody><tr>
            <td valign="top" width="132">
			<?php if ($logo): ?>
            <a href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home" id="logo"><img src="<?php print $logo; ?>" alt="<?php print t('Home'); ?>" /></a>
              <?php endif; ?>
      </td>
            <td width="50%" style="white-space: nowrap;" valign="top">
                <div class="mast"><?php if ($primary_links): ?>
   <a href="#main-menu"><?php print t('Jump to Navigation'); ?></a>
  <?php endif; ?></div>
            </td>
            <td align="right">
                <div id="toolbar"><span onClick="SignInBox.register({onSuccess: reloadPage});" class="clickable"><strong>Зарегистрироваться</strong></span> - <span onClick="SignInBox.signIn({onSuccess: reloadPage});" class="clickable">Войти</span></div><br>
            </td>
            <td id="header_search" width="1%" align="right">
                <div id="searchbar">

<table class="search_box" cellpadding="0" cellspacing="0">
  <tbody><tr>
    <td><?php if ($search_box): ?><div class="block block-theme"><?php print $search_box ?></div><?php endif; ?></td>
  </tr>
</tbody></table>
    
</div>
            </td>
        </tr>
    </tbody></table>
</div>
<?php if ($show_messages && $messages): print $messages; endif; ?>
<div id="shop_page" trackcontext="shop"><div id="left" style="width: 232px;">
<?php if ($search_box): ?><div class="block block-theme"><?php print $search_box ?></div><?php endif; ?>
          <?php print $left ?>

</div><div id="right" style="width:45%">
<?php print $content ?>
</div></div><div class="clear" id="footer_slim_placeholder"><?php print $footer_message . $footer ?></div><div id="footer">
<div id="footer_toolbar">Copyright © 2011 Fashionroom - <a href="">Условия Соглашения</a> - <a href="">Политика конфиденциальности</a> </div></div>
    
    
    <div class="accontainer" classname="accontainer"><ul></ul></div><?php print $closure; ?></body></html>
