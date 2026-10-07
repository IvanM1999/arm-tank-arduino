document.addEventListener('DOMContentLoaded', () => {
    const navButtons = document.querySelectorAll('.nav-btn');
    const appContent = document.getElementById('app-content');

    // Função para carregar o módulo HTML de forma assíncrona
    async function loadModule(moduleName) {
        try {
            // Adiciona classe de transição de saída
            appContent.classList.add('fade-out');
            
            const response = await fetch(`modulos/${moduleName}.html`);
            if (!response.ok) throw new Error('Erro ao carregar o módulo');
            
            const htmlText = await response.text();
            
            // Espera a animação de saída terminar antes de trocar o conteúdo
            setTimeout(() => {
                appContent.innerHTML = htmlText;
                appContent.classList.remove('fade-out');
                appContent.classList.add('fade-in');
                
                // Inicializa as funções lógicas específicas do módulo carregado
                initModuleLogic(moduleName);
            }, 300);

        } catch (error) {
            console.error(error);
            appContent.innerHTML = `<p class="error">Erro ao carregar o conteúdo. Tente novamente.</p>`;
        }
    }

    // Eventos de clique nos botões
    navButtons.forEach(button => {
        button.addEventListener('click', (e) => {
            const target = e.target.getAttribute('data-target');
            loadModule(target);
        });
    });

    // Função para ativar o código interativo de cada módulo injetado
    function initModuleLogic(moduleName) {
        if (moduleName === 'modulo-e') {
            setupLogicE();
        }
        // Outros módulos virão aqui...
    }
});
