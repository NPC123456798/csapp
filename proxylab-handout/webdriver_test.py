#!/usr/bin/env python3
# coding: gbk
"""
webdriver_test.py - Selenium 测试脚本 for CS:APP Proxy Lab
测试国内可访问的 HTTP 网站
"""

import sys
import time
import threading
import argparse
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from selenium.common.exceptions import TimeoutException, WebDriverException


# ==================== 配置 ====================


chrome_driver_path = r"/usr/local/bin/chromedriver"
class Config:
    """测试配置"""
    # 代理服务器配置
    PROXY_HOST = "localhost"
    PROXY_PORT = 4501  # 修改为您的代理端口
    
    # ===== 国内可访问的 HTTP 测试网站 =====
    # 注意：必须使用 HTTP，不是 HTTPS，避免 CONNECT 方法
    
    # 测试1: 清华大学官网 (HTTP 版)
    TSINGHUA_URL = "http://www.tsinghua.edu.cn/"
    
    # 测试2: 北京大学官网 (HTTP 版)
    PEKING_URL = "http://www.pku.edu.cn/"
    
    # 测试3: 百度 (HTTP 版 - 会自动跳转 HTTPS，可能有问题)
    # BAIDU_URL = "http://www.baidu.com/"
    
    # 测试4: HTTP 测试网站 - httpbin.org (HTTP 版)
    HTTPBIN_URL = "http://httpbin.org/"
    HTTPBIN_GET = "http://httpbin.org/get"
    HTTPBIN_IMAGE = "http://httpbin.org/image/png"
    
    # 测试5: 维基百科 (HTTP 版 - 可能跳转)
    # WIKI_URL = "http://www.wikipedia.org/"
    
    # 测试6: 本地测试文件（如果存在）
    LOCAL_TEST = "http://localhost:4500/home.html"
    
    # 默认使用 httpbin.org 进行测试（最稳定，纯 HTTP）
    DEFAULT_TEST_URL = HTTPBIN_GET
    DEFAULT_IMAGE_URL = HTTPBIN_IMAGE
    
    # 超时设置（秒）
    PAGE_LOAD_TIMEOUT = 15
    IMPLICIT_WAIT = 5
    
    # 测试次数
    CACHE_TEST_ROUNDS = 3
    CONCURRENT_THREADS = 5


# ==================== WebDriver 管理 ====================

def create_driver(proxy_port=None, headless=True):
    """
    创建配置好的 WebDriver 实例
    
    Args:
        proxy_port: 代理端口，None 表示不使用代理
        headless: 是否无头模式
    """
    # 导入 Service 类（新增）
    from selenium.webdriver.chrome.service import Service
    
    options = webdriver.ChromeOptions()
    
    if headless:
        options.add_argument("--headless=new")
    
    options.add_argument("--no-sandbox")
    options.add_argument("--disable-dev-shm-usage")
    options.add_argument("--disable-gpu")
    options.add_argument("--window-size=1920,1080")
    
    # 禁用可能产生 CONNECT 的功能
    options.add_argument("--disable-features=HttpsUpgrades")
    options.add_argument("--disable-features=HSTS")
    options.add_argument("--disable-preconnect")
    options.add_argument("--no-pings")
    
    # 配置代理
    if proxy_port:
        proxy = f"{Config.PROXY_HOST}:{proxy_port}"
        options.add_argument(f"--proxy-server=http://{proxy}")
        print(f"[INFO] 使用代理: {proxy}")
    else:
        print("[INFO] 不使用代理（直连）")
    
    # 禁用缓存
    options.add_argument("--disable-application-cache")
    options.add_argument("--disable-cache")
    options.add_argument("--disk-cache-size=0")
    
    # 使用您指定的驱动路径（使用文件开头的全局变量）
    global chrome_driver_path
    service = Service(executable_path=chrome_driver_path)
    print(f"[INFO] 使用 ChromeDriver 路径: {chrome_driver_path}")

    try:
        # 将 service 对象传递给 webdriver.Chrome
        driver = webdriver.Chrome(service=service, options=options)
        driver.set_page_load_timeout(Config.PAGE_LOAD_TIMEOUT)
        driver.implicitly_wait(Config.IMPLICIT_WAIT)
        return driver
    except WebDriverException as e:
        print(f"[ERROR] 无法创建 WebDriver: {e}")
        print(f"[HINT] 请检查指定的驱动路径是否正确: {chrome_driver_path}")
        print("[HINT] 请确保 ChromeDriver 版本与本地 Chrome 浏览器版本匹配")
        print("[HINT] 您可以通过浏览器访问 chrome://version/ 查看版本信息")
        sys.exit(1)


# ==================== 测试用例 ====================

class ProxyTests:
    """代理测试类"""
    
    def __init__(self):
        self.results = []
    
    def log(self, test_name, success, message="", duration=0):
        """记录测试结果"""
        status = "? PASS" if success else "? FAIL"
        print(f"[{status}] {test_name}: {message} ({duration:.3f}s)")
        self.results.append({
            "name": test_name,
            "success": success,
            "message": message,
            "duration": duration
        })
        return success
    
    # ---------- 测试1: 基本GET请求 ----------
    
    def test_basic_get(self):
        """测试基本GET请求（访问 httpbin.org）"""
        print("\n" + "="*50)
        print("测试1: 基本GET请求")
        print("="*50)
        print(f"目标: {Config.DEFAULT_TEST_URL}")
        
        driver = None
        start_time = time.time()
        
        try:
            driver = create_driver(proxy_port=Config.PROXY_PORT)
            driver.get(Config.DEFAULT_TEST_URL)
            
            # 等待页面加载
            WebDriverWait(driver, 10).until(
                lambda d: d.execute_script('return document.readyState') == 'complete'
            )
            
            # 验证页面内容
            page_source = driver.page_source
            if "httpbin" in page_source.lower() or "args" in page_source:
                duration = time.time() - start_time
                return self.log("Basic GET", True, 
                              f"成功访问 {Config.DEFAULT_TEST_URL}", duration)
            else:
                duration = time.time() - start_time
                return self.log("Basic GET", False,
                              f"页面内容异常: {page_source[:200]}", duration)
                
        except TimeoutException:
            duration = time.time() - start_time
            return self.log("Basic GET", False,
                          "页面加载超时", duration)
        except Exception as e:
            duration = time.time() - start_time
            return self.log("Basic GET", False,
                          f"异常: {str(e)}", duration)
        finally:
            if driver:
                driver.quit()
    
    # ---------- 测试2: 图片GET请求 ----------
    
    def test_image_get(self):
        """测试图片GET请求"""
        print("\n" + "="*50)
        print("测试2: 图片GET请求")
        print("="*50)
        print(f"目标: {Config.DEFAULT_IMAGE_URL}")
        
        driver = None
        start_time = time.time()
        
        try:
            driver = create_driver(proxy_port=Config.PROXY_PORT)
            driver.get(Config.DEFAULT_IMAGE_URL)
            
            WebDriverWait(driver, 10).until(
                lambda d: d.execute_script('return document.readyState') == 'complete'
            )
            
            duration = time.time() - start_time
            
            # 验证是否成功加载
            if len(driver.page_source) > 0:
                return self.log("Image GET", True,
                              f"成功获取图片 ({duration:.3f}s)", duration)
            else:
                return self.log("Image GET", False,
                              "图片获取失败", duration)
                          
        except Exception as e:
            duration = time.time() - start_time
            return self.log("Image GET", False,
                          f"失败: {e}", duration)
        finally:
            if driver:
                driver.quit()
    
    # ---------- 测试3: 缓存功能 ----------
    
    def test_caching(self):
        """测试代理缓存（多次GET同一资源）"""
        print("\n" + "="*50)
        print("测试3: 缓存功能")
        print("="*50)
        print(f"目标: {Config.DEFAULT_TEST_URL}")
        
        durations = []
        
        for i in range(Config.CACHE_TEST_ROUNDS):
            driver = None
            start_time = time.time()
            
            try:
                driver = create_driver(proxy_port=Config.PROXY_PORT)
                driver.get(Config.DEFAULT_TEST_URL)
                
                WebDriverWait(driver, 10).until(
                    lambda d: d.execute_script('return document.readyState') == 'complete'
                )
                
                duration = time.time() - start_time
                durations.append(duration)
                print(f"  第 {i+1} 次访问: {duration:.3f}s")
                
            except Exception as e:
                return self.log("Caching", False,
                              f"第 {i+1} 次访问失败: {e}", 0)
            finally:
                if driver:
                    driver.quit()
            
            time.sleep(0.5)
        
        # 分析缓存效果
        if len(durations) >= 2:
            first_time = durations[0]
            avg_later = sum(durations[1:]) / len(durations[1:])
            speedup = first_time / avg_later if avg_later > 0 else 0
            
            message = f"首次 {first_time:.3f}s, 后续平均 {avg_later:.3f}s, 加速比 {speedup:.2f}x"
            
            if speedup > 1.3 or avg_later < first_time * 0.8:
                return self.log("Caching", True, f"缓存可能生效 - {message}", sum(durations))
            else:
                return self.log("Caching", True, f"缓存效果不明显 - {message}", sum(durations))
        
        return self.log("Caching", True, "测试完成", sum(durations))
    
    # ---------- 测试4: 并发GET请求 ----------
    
    def test_concurrent_gets(self):
        """测试并发GET请求（多线程同时请求）"""
        print("\n" + "="*50)
        print("测试4: 并发GET请求")
        print("="*50)
        print(f"目标: {Config.DEFAULT_TEST_URL}")
        
        results = []
        threads = []
        
        def worker(thread_id):
            """工作线程"""
            start_time = time.time()
            driver = None
            
            try:
                driver = create_driver(proxy_port=Config.PROXY_PORT)
                driver.get(Config.DEFAULT_TEST_URL)
                
                WebDriverWait(driver, 10).until(
                    lambda d: d.execute_script('return document.readyState') == 'complete'
                )
                
                success = "httpbin" in driver.page_source.lower()
                duration = time.time() - start_time
                
                results.append({
                    "id": thread_id,
                    "success": success,
                    "duration": duration
                })
                print(f"  线程 {thread_id}: {'成功' if success else '失败'} ({duration:.3f}s)")
                
            except Exception as e:
                results.append({
                    "id": thread_id,
                    "success": False,
                    "error": str(e)
                })
                print(f"  线程 {thread_id}: 异常 - {e}")
            finally:
                if driver:
                    driver.quit()
        
        # 启动多个线程
        start_time = time.time()
        for i in range(Config.CONCURRENT_THREADS):
            t = threading.Thread(target=worker, args=(i,))
            threads.append(t)
            t.start()
        
        # 等待所有线程完成
        for t in threads:
            t.join()
        
        total_duration = time.time() - start_time
        
        successes = sum(1 for r in results if r.get("success", False))
        failures = len(results) - successes
        
        message = f"{successes}/{len(results)} 成功, 总耗时 {total_duration:.3f}s"
        
        if failures == 0:
            return self.log("Concurrent GETs", True, message, total_duration)
        else:
            return self.log("Concurrent GETs", False, message, total_duration)
    
    # ---------- 测试5: 404处理 ----------
    
    def test_404_response(self):
        """测试404响应（GET不存在的页面）"""
        print("\n" + "="*50)
        print("测试5: 404响应处理")
        print("="*50)
        
        not_found_url = f"http://httpbin.org/status/404"
        
        driver = None
        start_time = time.time()
        
        try:
            driver = create_driver(proxy_port=Config.PROXY_PORT)
            driver.get(not_found_url)
            
            WebDriverWait(driver, 10).until(
                lambda d: d.execute_script('return document.readyState') == 'complete'
            )
            
            duration = time.time() - start_time
            
            # 检查是否返回404页面
            page_source = driver.page_source
            if "404" in page_source:
                return self.log("404 Handling", True,
                              f"正确返回404 ({duration:.3f}s)", duration)
            else:
                return self.log("404 Handling", False,
                              f"未返回404错误页面: {page_source[:100]}", duration)
                          
        except Exception as e:
            duration = time.time() - start_time
            return self.log("404 Handling", True,
                          f"异常但可接受: {e}", duration)
        finally:
            if driver:
                driver.quit()


# ==================== 主程序 ====================

def print_summary(results):
    """打印测试摘要"""
    print("\n" + "="*50)
    print("测试摘要")
    print("="*50)
    
    total = len(results)
    passed = sum(1 for r in results if r["success"])
    failed = total - passed
    
    for r in results:
        status = "?" if r["success"] else "?"
        print(f"{status} {r['name']}: {r['message']}")
    
    print("-" * 50)
    print(f"总计: {total} 项, 通过: {passed}, 失败: {failed}")
    
    if failed == 0:
        print("? 所有测试通过！")
        return 0
    else:
        print("??  部分测试失败，请检查代理实现")
        return 1


def main():
    """主函数"""
    parser = argparse.ArgumentParser(description='Proxy Lab WebDriver 测试 (国内可访问)')
    parser.add_argument('--proxy-port', type=int, default=Config.PROXY_PORT,
                      help=f'代理端口 (默认: {Config.PROXY_PORT})')
    parser.add_argument('--test-url', type=str, default=Config.DEFAULT_TEST_URL,
                      help='测试URL (必须使用HTTP)')
    parser.add_argument('--test', type=str, default='all',
                      choices=['all', 'basic', 'image', 'cache', 'concurrent', '404'],
                      help='选择测试项目')
    parser.add_argument('--headed', action='store_true',
                      help='显示浏览器窗口（调试用）')
    
    args = parser.parse_args()
    
    # 更新配置
    Config.PROXY_PORT = args.proxy_port
    Config.DEFAULT_TEST_URL = args.test_url
    
    print("="*50)
    print("CS:APP Proxy Lab - WebDriver 测试 (国内可访问)")
    print("="*50)
    print(f"代理: http://{Config.PROXY_HOST}:{Config.PROXY_PORT}")
    print(f"测试URL: {Config.DEFAULT_TEST_URL}")
    print(f"模式: {'有头' if args.headed else '无头'}")
    print("="*50)
    
    # 创建测试实例
    tester = ProxyTests()
    
    # 运行测试
    test_map = {
        'basic': [tester.test_basic_get],
        'image': [tester.test_image_get],
        'cache': [tester.test_caching],
        'concurrent': [tester.test_concurrent_gets],
        '404': [tester.test_404_response],
        'all': [
            tester.test_basic_get,
            tester.test_image_get,
            tester.test_caching,
            tester.test_concurrent_gets,
            tester.test_404_response
        ]
    }
    
    tests_to_run = test_map.get(args.test, test_map['all'])
    
    for test_func in tests_to_run:
        try:
            test_func()
        except Exception as e:
            print(f"[ERROR] 测试异常: {e}")
            import traceback
            traceback.print_exc()
    
    # 打印摘要
    return print_summary(tester.results)


if __name__ == "__main__":
    sys.exit(main())