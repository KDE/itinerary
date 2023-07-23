#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.0-or-later
# SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>

from itinerarytestcase import ItineraryTestCase

import unittest
from appium import webdriver
from appium.webdriver.common.appiumby import AppiumBy

import os

# Online import form tests
class ItineraryTest(ItineraryTestCase):
    def test_db(self):
        self.triggerImportAction("Deutsche Bahn Online Ticket...")
        searchBtn = self.driver.find_element(by=AppiumBy.NAME, value="Search...")
        self.assertFalse(searchBtn.is_enabled())
        nameInput = self.driver.find_element(by=AppiumBy.NAME, value="Family name")
        nameInput.send_keys("Konqi")
        self.assertFalse(searchBtn.is_enabled())
        refInput = self.driver.find_element(by=AppiumBy.NAME, value="Order number or booking reference")
        refInput.send_keys("1234567")
        self.assertFalse(searchBtn.is_enabled())
        refInput.send_keys("12345")
        self.assertTrue(searchBtn.is_enabled())
        refInput.clear()
        self.assertFalse(searchBtn.is_enabled())
        refInput.send_keys("ABC123")
        self.assertTrue(searchBtn.is_enabled())
        self.goBack()

    def test_sncf(self):
        self.triggerImportAction("SNCF Online Ticket...")
        searchBtn = self.driver.find_element(by=AppiumBy.NAME, value="Search...")
        self.assertFalse(searchBtn.is_enabled())
        nameInput = self.driver.find_element(by=AppiumBy.NAME, value="Family name")
        nameInput.send_keys("Konqi")
        self.assertFalse(searchBtn.is_enabled())
        refInput = self.driver.find_element(by=AppiumBy.NAME, value="Booking reference")
        refInput.send_keys("ABC123")
        self.assertTrue(searchBtn.is_enabled())
        self.goBack()

if __name__ == '__main__':
    unittest.main()
