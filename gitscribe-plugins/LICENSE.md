# MIT License

Copyright (c) 2025 GitScribe

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

## Why MIT License for the Plugin SDK?

We chose the MIT License for the plugin SDK to:

1. **Encourage ecosystem growth**: No restrictions on commercial or non-commercial use
2. **Lower barrier to entry**: Developers don't need to worry about license compliance
3. **Allow proprietary plugins**: Developers can create closed-source plugins
4. **Build trust**: Permissive licensing shows commitment to the community
5. **Maximize adoption**: MIT is widely understood and accepted

## What This Means for Plugin Developers

### ✅ You CAN:

- Use this SDK to create free plugins
- Use this SDK to create paid/commercial plugins
- Modify the SDK for your needs
- Redistribute the SDK
- Keep your plugin source code private
- Sell plugins without any revenue sharing*
- Create competing plugin ecosystems

*Note: The GitScribe plugin registry may take a commission on paid plugins sold through the marketplace. Direct sales are unrestricted.

### 📝 You MUST:

- Include the MIT license text in substantial portions of the SDK code
- Include the copyright notice

### ❌ You DON'T HAVE TO:

- Open source your plugins
- Share modifications to the SDK
- Pay licensing fees
- Attribute GitScribe in your plugin (though appreciated!)

## Plugins vs. Core Application

**Important**: While the plugin SDK is MIT licensed, the core GitScribe application (gitscribe-app, gitscribe-shell) is proprietary. This SDK license only covers:

- Plugin API definitions (`@gitscribe/plugin-api`)
- Plugin CLI tools (`@gitscribe/plugin-cli`)
- Plugin development utilities (`@gitscribe/plugin-dev`)
- Plugin examples and templates
- Plugin documentation

It does NOT cover:
- GitScribe application binaries
- GitScribe core library (MPL-2.0)
- GitScribe shell extension (Proprietary)
- GitScribe branding and trademarks

## Third-Party Licenses

This SDK may include third-party dependencies under their own licenses. See individual package.json files for details.

## Contributing

By contributing to this SDK, you agree to license your contributions under the MIT License.

## Questions?

For licensing questions: licensing@gitscribe.dev
For plugin development help: developers@gitscribe.dev
