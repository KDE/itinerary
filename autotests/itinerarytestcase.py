#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.0-or-later
# SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>

import unittest
from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from appium.webdriver.webdriver import ExtensionBase
import selenium.common.exceptions
from selenium.webdriver.common.action_chains import ActionChains
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support.ui import WebDriverWait

import os


class SetValueCommand(ExtensionBase):

    def method_name(self):
        return 'set_value'

    def set_value(self, element, value: str):
        """
        Set the value on this element in the application
        Args:
            value: The value to be set
        """
        data = {
            'id': element.id,
            'text': value,
        }
        return self.execute(data)['value']

    def add_command(self):
        return 'post', '/session/$sessionId/appium/element/$id/value'


# common base class for Appium tests
class ItineraryTestCase(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        options = AppiumOptions()
        options.set_capability("app", "org.kde.itinerary.desktop")
        self.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options, extensions=[SetValueCommand])
        self.driver.implicitly_wait = 10

        # skip the welcome screen
        welcomeBtn = self.driver.find_element(by=AppiumBy.NAME, value="Got it!")
        welcomeBtn.click()

    @classmethod
    def tearDownClass(self):
        self.driver.quit()

    # helper methods
    def goBack(self):
        self.driver.find_element(by=AppiumBy.NAME, value="Navigate Back").click()

    def openFile(self, fileName):
        actions = ActionChains(self.driver)
        actions.send_keys(os.path.join(os.path.dirname(__file__), fileName)).perform()
        actions.send_keys(Keys.RETURN).perform()

    def openGlobalDrawer(self):
        self.driver.find_element(by=AppiumBy.NAME, value="Open drawer").click()

    def openContextDrawer(self):
        self.driver.find_elements(by=AppiumBy.NAME, value="Open drawer")[1].click()

    def triggerImportAction(self, name):
        self.openGlobalDrawer()
        self.driver.find_element(by=AppiumBy.NAME, value="Import...").click()
        action = self.driver.find_element(by=AppiumBy.NAME, value=name)
        self.assertTrue(action.is_enabled())
        action.click()
